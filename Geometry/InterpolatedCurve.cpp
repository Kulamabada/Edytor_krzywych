#include "InterpolatedCurve.h"
#include "Point.h"
#include "../Math/IMathProvider.h"
#include <cmath>
#include <limits>
#include <algorithm>

InterpolatedCurve::InterpolatedCurve(std::shared_ptr<IMathProvider> mathProvider,
                                    InterpolationType type)
    : type(type),
      mathProvider(std::move(mathProvider)){}

void InterpolatedCurve::addPoint(const Point& point) {
    points.push_back(point);
    needsRebuild = true;
}

void InterpolatedCurve::insertPoint(size_t index, const Point& point) {
    if (index <= points.size()) {
        points.insert(points.begin() + index, point);
        needsRebuild = true;
    }
}

void InterpolatedCurve::removePoint(size_t index) {
    if (index < points.size()) {
        points.erase(points.begin() + index);
        needsRebuild = true;
    }
}

void InterpolatedCurve::setPoints(const std::vector<Point>& newPoints) {
    points = newPoints;
    needsRebuild = true;
}

void InterpolatedCurve::movePoint(size_t index, const Point& newPosition) {
    if (index < points.size()) {
        points[index] = newPosition;
        needsRebuild = true;
    }
}

const std::vector<Point>& InterpolatedCurve::getPoints() const {
    return points;
}

size_t InterpolatedCurve::getPointCount() const {
    return points.size();
}

void InterpolatedCurve::setInterpolationType(InterpolationType newType) {
    if (type != newType) {
        type = newType;
        needsRebuild = true;
    }
}

InterpolatedCurve::InterpolationType InterpolatedCurve::getInterpolationType() const { return type; }

void InterpolatedCurve::rebuildInterpolation() const {
    if (!needsRebuild || points.size() < 2) {
        needsRebuild = false;
        return;
    }
    coefficients.clear();

    switch (type) {
        case InterpolationType::NIFS3:
            buildNIFS3();
        break;
        case InterpolationType::OIFS3:
            buildOIFS3();
        break;
        case InterpolationType::Lagrange:
            buildLagrange();
        break;
        case InterpolationType::Barycentric:
            buildBarycentric();
        break;
    }

    needsRebuild = false;
}

void InterpolatedCurve::buildNIFS3() const {
    const size_t n = points.size();
    if (n < 2) return;

    // Parametryzacja od 0 do 1
    std::vector<double> xs(n), ys_x(n), ys_y(n);
    for (size_t i = 0; i < n; ++i) {
        xs[i] = static_cast<double>(i) / (n - 1);
        ys_x[i] = points[i].x();
        ys_y[i] = points[i].y();
    }

    // Obliczanie ilorazów różnicowych
    std::vector<double> ds_x;
    std::vector<double> ds_y;
    for (size_t i = 1; i < n - 1; ++i) {
        const double term1_x = (ys_x[i + 1] - ys_x[i]) / (xs[i + 1] - xs[i]);
        const double term2_x = (ys_x[i] - ys_x[i - 1]) / (xs[i] - xs[i - 1]);
        const double term1_y = (ys_y[i + 1] - ys_y[i]) / (xs[i + 1] - xs[i]);
        const double term2_y = (ys_y[i] - ys_y[i - 1]) / (xs[i] - xs[i - 1]);
        ds_x.push_back(6.0 * (term1_x - term2_x) / (xs[i + 1] - xs[i - 1]));
        ds_y.push_back(6.0 * (term1_y - term2_y) / (xs[i + 1] - xs[i - 1]));
    }

    // Współczynniki p
    std::vector<double> ps;
    ps.push_back(0.0);
    for (size_t i = 1; i < n - 1; ++i) {
        double lambda = (xs[i] - xs[i - 1]) / (xs[i + 1] - xs[i - 1]);
        ps.push_back((lambda - 1.0) / (lambda * ps[i - 1] + 2.0));
    }

    // Współczynniki q dla x i y
    std::vector<double> qs_x;
    std::vector<double> qs_y;
    qs_x.push_back(0.0);
    qs_y.push_back(0.0);
    for (size_t i = 1; i < n - 1; ++i) {
        const double lambda = (xs[i] - xs[i - 1]) / (xs[i + 1] - xs[i - 1]);
        qs_x.push_back((ds_x[i - 1] - lambda * qs_x[i - 1]) / (lambda * ps[i - 1] + 2.0));
        qs_y.push_back((ds_y[i - 1] - lambda * qs_y[i - 1]) / (lambda * ps[i - 1] + 2.0));
    }

    // Momenty M - dokładnie jak w Pythonie
    std::vector<double> M_x(n, 0.0), M_y(n, 0.0);

    // Uzupełniamy wartości od końca, zgodnie z algorytmem Pythona
    M_x[n-1] = 0.0;  // Naturalne warunki brzegowe
    M_y[n-1] = 0.0;
    M_x[n-2] = qs_x.back();
    M_y[n-2] = qs_y.back();

    for (int i = n - 3; i > 0; --i) {
        M_x[i] = ps[i] * M_x[i+1] + qs_x[i];
        M_y[i] = ps[i] * M_y[i+1] + qs_y[i];
    }
    // M_x[0] i M_y[0] pozostają równe 0 (naturalne warunki brzegowe)

    // Obliczanie współczynników wielomianów
    coefficients.clear();
    for (size_t i = 0; i < n - 1; ++i) {
        double h = xs[i + 1] - xs[i];
        Eigen::Matrix<double, 4, 2> coef;

        // Obliczamy współczynniki dla wielomianu w formie standardowej
        // a + b*t + c*t^2 + d*t^3
        coef(0, 0) = ys_x[i];
        coef(1, 0) = (ys_x[i + 1] - ys_x[i]) - (h * h / 6.0) * (M_x[i + 1] + 2.0 * M_x[i]);
        coef(2, 0) = (h * h / 2.0) * M_x[i];
        coef(3, 0) = (h * h / 6.0) * (M_x[i + 1] - M_x[i]);

        coef(0, 1) = ys_y[i];
        coef(1, 1) = (ys_y[i + 1] - ys_y[i]) - (h * h / 6.0) * (M_y[i + 1] + 2.0 * M_y[i]);
        coef(2, 1) = (h * h / 2.0) * M_y[i];
        coef(3, 1) = (h * h / 6.0) * (M_y[i + 1] - M_y[i]);

        coefficients.push_back(coef);
    }
}

void InterpolatedCurve::buildOIFS3() const {
    const size_t n = points.size();
    if (n < 3) return;

    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(n, n);
    Eigen::VectorXd bx = Eigen::VectorXd::Zero(n);
    Eigen::VectorXd by = Eigen::VectorXd::Zero(n);

    for (size_t i = 0; i < n; ++i) {
        size_t prev = (i > 0) ? (i - 1) : (n - 1);
        size_t next = (i < n - 1) ? (i + 1) : 0;

        A(i, prev) = 1.0;
        A(i, i) = 4.0;
        A(i, next) = 1.0;

        bx(i) = 3.0 * (points[next].x() - points[prev].x());
        by(i) = 3.0 * (points[next].y() - points[prev].y());
    }

    Eigen::VectorXd cx = A.colPivHouseholderQr().solve(bx);
    Eigen::VectorXd cy = A.colPivHouseholderQr().solve(by);

    for (size_t i = 0; i < n; ++i) {
        size_t next = (i < n - 1) ? (i + 1) : 0;

        Eigen::Matrix<double, 4, 2> coef;

        coef(0, 0) = points[i].x();
        coef(1, 0) = cx(i);
        coef(2, 0) = 3.0 * (points[next].x() - points[i].x()) - 2.0 * cx(i) - cx(next);
        coef(3, 0) = 2.0 * (points[i].x() - points[next].x()) + cx(i) + cx(next);

        coef(0, 1) = points[i].y();
        coef(1, 1) = cy(i);
        coef(2, 1) = 3.0 * (points[next].y() - points[i].y()) - 2.0 * cy(i) - cy(next);
        coef(3, 1) = 2.0 * (points[i].y() - points[next].y()) + cy(i) + cy(next);

        coefficients.push_back(coef);
    }
}

void InterpolatedCurve::buildLagrange() const {
    const size_t n = points.size();
    if (n < 2) return;
}

void InterpolatedCurve::buildBarycentric() const {
    const size_t n = points.size();
    if (n < 2) return;
}

Vector InterpolatedCurve::getTangentAt(double t) const {
    const double epsilon = 1e-6;
    Point p1 = getPointAt(t);
    Point p2 = getPointAt(t + epsilon);

    return Vector(
        (p2.x() - p1.x()) / epsilon,
        (p2.y() - p1.y()) / epsilon,
        (p2.z() - p1.z()) / epsilon
    );
}

Point InterpolatedCurve::getPointAt(double t) const {
    if (points.empty()) return Point(0.0, 0.0, 0.0);
    if (points.size() == 1) return points[0];

    if (needsRebuild) {
        rebuildInterpolation();
    }

    t = std::max(0.0, std::min(1.0, t));

    size_t numSegments = (type == InterpolationType::OIFS3) ? points.size() : (points.size() - 1);
    double segmentLength = 1.0 / numSegments;
    size_t segmentIndex = std::min(static_cast<size_t>(t / segmentLength), numSegments - 1);
    double localT = (t - segmentIndex * segmentLength) / segmentLength;

    if (type == InterpolationType::NIFS3 || type == InterpolationType::OIFS3) {
        if (segmentIndex >= coefficients.size())
            return points[points.size() - 1];

        const auto& coef = coefficients[segmentIndex];
        Eigen::Vector4d tVec;
        tVec << 1.0, localT, localT*localT, localT*localT*localT;

        double x = tVec.dot(coef.col(0));
        double y = tVec.dot(coef.col(1));

        return Point(x, y, 0.0);
    }
    else if (type == InterpolationType::Lagrange) {
        double x = 0.0;
        double y = 0.0;

        for (size_t i = 0; i < points.size(); ++i) {
            double L = 1.0;
            for (size_t j = 0; j < points.size(); ++j) {
                if (i != j) {
                    double tj = static_cast<double>(j) / (points.size() - 1);
                    double ti = static_cast<double>(i) / (points.size() - 1);
                    L *= (t - tj) / (ti - tj);
                }
            }
            x += points[i].x() * L;
            y += points[i].y() * L;
        }

        return Point(x, y, 0.0);
    }
    else if (type == InterpolationType::Barycentric) {
        double x = 0.0;
        double y = 0.0;
        double sumW = 0.0;

        std::vector<double> weights(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            double ti = static_cast<double>(i) / (points.size() - 1);
            weights[i] = 1.0;
            for (size_t j = 0; j < points.size(); ++j) {
                if (i != j) {
                    double tj = static_cast<double>(j) / (points.size() - 1);
                    weights[i] *= (ti - tj);
                }
            }
            weights[i] = 1.0 / weights[i];
        }

        for (size_t i = 0; i < points.size(); ++i) {
            double ti = static_cast<double>(i) / (points.size() - 1);

            if (std::abs(t - ti) < 1e-10) {
                return points[i];
            }

            double w = weights[i] / (t - ti);
            x += points[i].x() * w;
            y += points[i].y() * w;
            sumW += w;
        }
        return Point(x / sumW, y / sumW, 0.0);
    }

    size_t nextIndex = (segmentIndex + 1) % points.size();
    double oneMinusT = 1.0 - localT;
    double x = oneMinusT * points[segmentIndex].x() + localT * points[nextIndex].x();
    double y = oneMinusT * points[segmentIndex].y() + localT * points[nextIndex].y();
    double z = oneMinusT * points[segmentIndex].z() + localT * points[nextIndex].z();

    return Point(x, y, z);
}



double InterpolatedCurve::getLength() const {
    constexpr int steps = 200;
    double length = 0.0;
    Point prev = getPointAt(0.0);

    for (int i = 1; i <= steps; ++i) {
        const double t = static_cast<double>(i) / steps;
        Point curr = getPointAt(t);
        length += prev.distanceTo(curr);
        prev = curr;
    }

    return length;
}

void InterpolatedCurve::transform(const TransformMatrix& matrix) {
    for (auto& point : points) {
        point.transform(matrix);
    }
    needsRebuild = true;
}

std::unique_ptr<Curve> InterpolatedCurve::clone() const {
    auto copy = std::make_unique<InterpolatedCurve>(mathProvider, type);
    copy->setPoints(points);
    return copy;
}

Vector InterpolatedCurve::getPosition() const {
    if (points.empty()) return Vector::Zero();

    Vector sum = Vector::Zero();
    for (const auto& p : points) {
        sum += Vector(p.x(), p.y(), p.z());
    }
    return sum / static_cast<double>(points.size());
}

void InterpolatedCurve::setPosition(const Eigen::Vector3d& position) {
    if (points.empty()) return;

    Vector delta = position - getPosition();
    for (auto& p : points) {
        p.setX(p.x() + delta.x());
        p.setY(p.y() + delta.y());
        p.setZ(p.z() + delta.z());
    }
    needsRebuild = true;
}
