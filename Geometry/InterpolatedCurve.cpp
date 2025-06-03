#include "InterpolatedCurve.h"

InterpolatedCurve::InterpolatedCurve(std::shared_ptr<IMathProvider> mathProvider,
                                    InterpolationType type)
    : Curve(CurveType::Interpolated),
      type(type),
      mathProvider(std::move(mathProvider))
{
}

std::unique_ptr<Curve> InterpolatedCurve::createClone() const {
    return std::make_unique<InterpolatedCurve>(mathProvider, type);
}

void InterpolatedCurve::copySpecificPropertiesTo(Curve* target) const {
    if (auto interpolatedTarget = dynamic_cast<InterpolatedCurve*>(target)) {
        interpolatedTarget->type = type;
        interpolatedTarget->mathProvider = mathProvider;
    }
}

void InterpolatedCurve::onControlPointsChanged() {
    needsRebuild = true;
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
    if (!needsRebuild || m_controlPoints.size() < 2) {
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

Vector InterpolatedCurve::getTangentAt(double t) const {
    return calculateTangentByFiniteDifference(t);
}

Point InterpolatedCurve::getPointAt(double t) const {
    if (m_controlPoints.empty()) return Point(0.0, 0.0, 0.0);
    if (m_controlPoints.size() == 1) return m_controlPoints[0];

    if (needsRebuild) {
        rebuildInterpolation();
    }

    validateParameterRange(t);

    size_t numSegments = (type == InterpolationType::OIFS3) ? m_controlPoints.size() : (m_controlPoints.size() - 1);
    double segmentLength = 1.0 / numSegments;
    size_t segmentIndex = std::min(static_cast<size_t>(t / segmentLength), numSegments - 1);
    double localT = (t - segmentIndex * segmentLength) / segmentLength;

    if (type == InterpolationType::NIFS3 || type == InterpolationType::OIFS3) {
        if (segmentIndex >= coefficients.size())
            return m_controlPoints[m_controlPoints.size() - 1];

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

        for (size_t i = 0; i < m_controlPoints.size(); ++i) {
            double L = 1.0;
            for (size_t j = 0; j < m_controlPoints.size(); ++j) {
                if (i != j) {
                    double tj = static_cast<double>(j) / (m_controlPoints.size() - 1);
                    double ti = static_cast<double>(i) / (m_controlPoints.size() - 1);
                    L *= (t - tj) / (ti - tj);
                }
            }
            x += m_controlPoints[i].x() * L;
            y += m_controlPoints[i].y() * L;
        }

        return Point(x, y, 0.0);
    }
    else if (type == InterpolationType::Barycentric) {
        double x = 0.0;
        double y = 0.0;
        double sumW = 0.0;

        std::vector<double> weights(m_controlPoints.size());
        for (size_t i = 0; i < m_controlPoints.size(); ++i) {
            double ti = static_cast<double>(i) / (m_controlPoints.size() - 1);
            weights[i] = 1.0;
            for (size_t j = 0; j < m_controlPoints.size(); ++j) {
                if (i != j) {
                    double tj = static_cast<double>(j) / (m_controlPoints.size() - 1);
                    weights[i] *= (ti - tj);
                }
            }
            weights[i] = 1.0 / weights[i];
        }

        for (size_t i = 0; i < m_controlPoints.size(); ++i) {
            double ti = static_cast<double>(i) / (m_controlPoints.size() - 1);

            if (std::abs(t - ti) < 1e-10) {
                return m_controlPoints[i];
            }

            double w = weights[i] / (t - ti);
            x += m_controlPoints[i].x() * w;
            y += m_controlPoints[i].y() * w;
            sumW += w;
        }
        return {x / sumW, y / sumW, 0.0};
    }

    const size_t nextIndex = (segmentIndex + 1) % m_controlPoints.size();
    return interpolateLinear(m_controlPoints[segmentIndex], m_controlPoints[nextIndex], localT);
}

void InterpolatedCurve::buildNIFS3() const {
    const size_t n = m_controlPoints.size();
    if (n < 2) return;

    std::vector<double> xs(n), ys_x(n), ys_y(n);
    for (size_t i = 0; i < n; ++i) {
        xs[i] = static_cast<double>(i) / (n - 1);
        ys_x[i] = m_controlPoints[i].x();
        ys_y[i] = m_controlPoints[i].y();
    }

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

    std::vector<double> ps;
    ps.push_back(0.0);
    for (size_t i = 1; i < n - 1; ++i) {
        double lambda = (xs[i] - xs[i - 1]) / (xs[i + 1] - xs[i - 1]);
        ps.push_back((lambda - 1.0) / (lambda * ps[i - 1] + 2.0));
    }

    std::vector<double> qs_x;
    std::vector<double> qs_y;
    qs_x.push_back(0.0);
    qs_y.push_back(0.0);
    for (size_t i = 1; i < n - 1; ++i) {
        const double lambda = (xs[i] - xs[i - 1]) / (xs[i + 1] - xs[i - 1]);
        qs_x.push_back((ds_x[i - 1] - lambda * qs_x[i - 1]) / (lambda * ps[i - 1] + 2.0));
        qs_y.push_back((ds_y[i - 1] - lambda * qs_y[i - 1]) / (lambda * ps[i - 1] + 2.0));
    }

    std::vector<double> M_x(n, 0.0), M_y(n, 0.0);

    M_x[n-1] = 0.0;
    M_y[n-1] = 0.0;
    M_x[n-2] = qs_x.back();
    M_y[n-2] = qs_y.back();

    for (int i = n - 3; i > 0; --i) {
        M_x[i] = ps[i] * M_x[i+1] + qs_x[i];
        M_y[i] = ps[i] * M_y[i+1] + qs_y[i];
    }

    coefficients.clear();
    for (size_t i = 0; i < n - 1; ++i) {
        double h = xs[i + 1] - xs[i];
        Eigen::Matrix<double, 4, 2> coef;

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
    const size_t n = m_controlPoints.size();
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

        bx(i) = 3.0 * (m_controlPoints[next].x() - m_controlPoints[prev].x());
        by(i) = 3.0 * (m_controlPoints[next].y() - m_controlPoints[prev].y());
    }

    Eigen::VectorXd cx = A.colPivHouseholderQr().solve(bx);
    Eigen::VectorXd cy = A.colPivHouseholderQr().solve(by);

    for (size_t i = 0; i < n; ++i) {
        size_t next = (i < n - 1) ? (i + 1) : 0;

        Eigen::Matrix<double, 4, 2> coef;

        coef(0, 0) = m_controlPoints[i].x();
        coef(1, 0) = cx(i);
        coef(2, 0) = 3.0 * (m_controlPoints[next].x() - m_controlPoints[i].x()) - 2.0 * cx(i) - cx(next);
        coef(3, 0) = 2.0 * (m_controlPoints[i].x() - m_controlPoints[next].x()) + cx(i) + cx(next);

        coef(0, 1) = m_controlPoints[i].y();
        coef(1, 1) = cy(i);
        coef(2, 1) = 3.0 * (m_controlPoints[next].y() - m_controlPoints[i].y()) - 2.0 * cy(i) - cy(next);
        coef(3, 1) = 2.0 * (m_controlPoints[i].y() - m_controlPoints[next].y()) + cy(i) + cy(next);

        coefficients.push_back(coef);
    }
}

void InterpolatedCurve::buildLagrange() const {
    const size_t n = m_controlPoints.size();
    if (n < 2) return;
}

void InterpolatedCurve::buildBarycentric() const {
    const size_t n = m_controlPoints.size();
    if (n < 2) return;
}
