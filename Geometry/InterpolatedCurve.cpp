#include "InterpolatedCurve.h"
#include "Point.h"
#include "../Math/IMathProvider.h"
#include <cmath>

InterpolatedCurve::InterpolatedCurve(std::shared_ptr<IMathProvider> mathProvider,
                                    InterpolationType type)
    : mathProvider(std::move(mathProvider)),
      type(type),
      needsRebuild(true) {
}

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

InterpolatedCurve::InterpolationType InterpolatedCurve::getInterpolationType() const {
    return type;
}

void InterpolatedCurve::rebuildInterpolation() const {
    if (!needsRebuild || points.size() < 2) {
        needsRebuild = false;
        return;
    }

    coefficients.clear();

    switch (type) {
        case InterpolationType::CubicSpline:
            buildCubicSpline();
            break;
        case InterpolationType::CatmullRom:
            buildCatmullRom();
            break;
        case InterpolationType::Linear:
            // Dla interpolacji liniowej nie potrzebujemy współczynników
            break;
    }

    needsRebuild = false;
}

void InterpolatedCurve::buildCubicSpline() const {
    const size_t n = points.size();
    if (n < 2) return;

    // Utwórz trójdiagonalną macierz dla obliczania współczynników spline
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(n, n);
    Eigen::VectorXd bx = Eigen::VectorXd::Zero(n);
    Eigen::VectorXd by = Eigen::VectorXd::Zero(n);

    // Naturalne warunki brzegowe
    A(0, 0) = 1.0;
    A(n-1, n-1) = 1.0;

    // Wypełnianie macierzy dla punktów wewnętrznych
    for (size_t i = 1; i < n - 1; ++i) {
        A(i, i-1) = 1.0;
        A(i, i) = 4.0;
        A(i, i+1) = 1.0;

        bx(i) = 3.0 * (points[i+1].x() - points[i-1].x());
        by(i) = 3.0 * (points[i+1].y() - points[i-1].y());
    }

    // Rozwiązanie układu równań dla pochodnych
    Eigen::VectorXd cx = A.colPivHouseholderQr().solve(bx);
    Eigen::VectorXd cy = A.colPivHouseholderQr().solve(by);

    // Obliczenie współczynników dla każdego segmentu
    for (size_t i = 0; i < n - 1; ++i) {
        Eigen::Matrix<double, 4, 2> coef;

        // Współczynniki dla x
        coef(0, 0) = points[i].x();
        coef(1, 0) = cx(i);
        coef(2, 0) = 3.0 * (points[i+1].x() - points[i].x()) - 2.0 * cx(i) - cx(i+1);
        coef(3, 0) = 2.0 * (points[i].x() - points[i+1].x()) + cx(i) + cx(i+1);

        // Współczynniki dla y
        coef(0, 1) = points[i].y();
        coef(1, 1) = cy(i);
        coef(2, 1) = 3.0 * (points[i+1].y() - points[i].y()) - 2.0 * cy(i) - cy(i+1);
        coef(3, 1) = 2.0 * (points[i].y() - points[i+1].y()) + cy(i) + cy(i+1);

        coefficients.push_back(coef);
    }
}

void InterpolatedCurve::buildCatmullRom() const {
    if (points.size() < 2) return;

    // Macierz bazowa Catmull-Rom
    Eigen::Matrix4d basis;
    basis << 0, 1, 0, 0,
             -0.5, 0, 0.5, 0,
             1, -2.5, 2, -0.5,
             -0.5, 1.5, -1.5, 0.5;

    // Dla każdego segmentu
    for (size_t i = 0; i < points.size() - 1; ++i) {
        // Punkty kontrolne z ekstrapolacją na krańcach
        Point p0 = (i > 0) ? points[i-1] : points[i];
        Point p1 = points[i];
        Point p2 = points[i+1];
        Point p3 = (i+1 < points.size()-1) ? points[i+2] : points[i+1];

        // Macierz punktów kontrolnych
        Eigen::Matrix<double, 4, 2> controls;
        controls << p0.x(), p0.y(),
                   p1.x(), p1.y(),
                   p2.x(), p2.y(),
                   p3.x(), p3.y();

        // Obliczenie współczynników
        coefficients.push_back(basis * controls);
    }
}

Point InterpolatedCurve::getPointAt(double t) const {
    if (points.empty()) return Point(0.0, 0.0, 0.0);
    if (points.size() == 1) return points[0];

    // Aktualizacja danych interpolacji jeśli potrzebna
    if (needsRebuild) {
        rebuildInterpolation();
    }

    // Ograniczenie parametru t do [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Obliczenie segmentu i lokalnego parametru
    size_t numSegments = points.size() - 1;
    double segmentLength = 1.0 / numSegments;
    size_t segmentIndex = std::min(static_cast<size_t>(t / segmentLength), numSegments - 1);
    double localT = (t - segmentIndex * segmentLength) / segmentLength;

    if (type == InterpolationType::Linear) {
        // Prosta interpolacja liniowa
        double oneMinusT = 1.0 - localT;
        double x = oneMinusT * points[segmentIndex].x() + localT * points[segmentIndex + 1].x();
        double y = oneMinusT * points[segmentIndex].y() + localT * points[segmentIndex + 1].y();
        double z = oneMinusT * points[segmentIndex].z() + localT * points[segmentIndex + 1].z();
        return Point(x, y, z);
    } else {
        // Interpolacja krzywoliniowa
        if (segmentIndex >= coefficients.size()) {
            return points[segmentIndex]; // Awaryjnie
        }

        const auto& coef = coefficients[segmentIndex];
        Eigen::Vector4d tVec;
        tVec << 1.0, localT, localT*localT, localT*localT*localT;

        double x = tVec.dot(coef.col(0));
        double y = tVec.dot(coef.col(1));

        return Point(x, y, 0.0);
    }
}

Vector InterpolatedCurve::getTangentAt(double t) const {
    // Numeryczne obliczenie pochodnej
    const double epsilon = 1e-6;
    Point p1 = getPointAt(t);
    Point p2 = getPointAt(t + epsilon);

    return Vector(
        (p2.x() - p1.x()) / epsilon,
        (p2.y() - p1.y()) / epsilon,
        (p2.z() - p1.z()) / epsilon
    );
}

double InterpolatedCurve::getCurvatureAt(double t) const {
    // Obliczenie krzywizny przez przybliżenie drugiej pochodnej
    const double h = 1e-6;
    Vector first = getTangentAt(t);
    Vector second = (getTangentAt(t + h) - first) / h;

    double numerator = std::abs(first.x() * second.y() - first.y() * second.x());
    double denominator = std::pow(first.x() * first.x() + first.y() * first.y(), 1.5);

    return (denominator < 1e-10) ? 0.0 : (numerator / denominator);
}

bool InterpolatedCurve::isPointOnCurve(const Point& point, double tolerance) const {
    // Sprawdzenie przez próbkowanie krzywej
    const int steps = 50;
    for (int i = 0; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        if (getPointAt(t).distanceTo(point) <= tolerance) {
            return true;
        }
    }
    return false;
}

double InterpolatedCurve::getLength() const {
    // Przybliżenie długości przez sumowanie odcinków
    const int steps = 20;
    double length = 0.0;
    Point prev = getPointAt(0.0);

    for (int i = 1; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        Point curr = getPointAt(t);
        length += prev.distanceTo(curr);
        prev = curr;
    }

    return length;
}

std::vector<Point> InterpolatedCurve::getIntersectionsWith(const Curve& other) const {
    // Uproszczona implementacja - brute force z optymalizacją
    std::vector<Point> result;
    const int steps = 25;
    const double tolerance = 1e-4;

    for (int i = 0; i <= steps; ++i) {
        double t1 = static_cast<double>(i) / steps;
        Point p1 = getPointAt(t1);

        for (int j = 0; j <= steps; ++j) {
            double t2 = static_cast<double>(j) / steps;
            Point p2 = other.getPointAt(t2);

            if (p1.distanceTo(p2) < tolerance) {
                // Sprawdź czy punkt już istnieje w wynikach
                bool exists = false;
                for (const auto& p : result) {
                    if (p.distanceTo(p1) < tolerance) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    result.push_back(p1);
                }
                break;
            }
        }
    }

    return result;
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

Vector InterpolatedCurve::getScale() const {
    return Vector::Ones();
}

void InterpolatedCurve::setScale(const Vector& scale) {
    if (points.empty()) return;

    Vector center = getPosition();
    for (auto& p : points) {
        Vector relPos(p.x() - center.x(), p.y() - center.y(), p.z() - center.z());
        p.setX(center.x() + relPos.x() * scale.x());
        p.setY(center.y() + relPos.y() * scale.y());
        p.setZ(center.z() + relPos.z() * scale.z());
    }
    needsRebuild = true;
}

Eigen::Quaterniond InterpolatedCurve::getRotation() const {
    return Eigen::Quaterniond::Identity();
}

void InterpolatedCurve::setRotation(const Eigen::Quaterniond& rotation) {
    if (points.empty()) return;

    Vector center = getPosition();
    for (auto& p : points) {
        Vector relPos(p.x() - center.x(), p.y() - center.y(), p.z() - center.z());
        Vector rotated = rotation * relPos;
        p.setX(center.x() + rotated.x());
        p.setY(center.y() + rotated.y());
        p.setZ(center.z() + rotated.z());
    }
    needsRebuild = true;
}
