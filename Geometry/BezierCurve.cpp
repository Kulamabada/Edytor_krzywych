#include "BezierCurve.h"
#include "TransformMatrix.h"
#include <cmath>
#include <algorithm>

BezierCurve::BezierCurve() {
}

void BezierCurve::addControlPoint(const Point& point) {
    m_controlPoints.push_back(point);
}

void BezierCurve::insertControlPoint(size_t index, const Point& point) {
    if (index <= m_controlPoints.size()) {
        m_controlPoints.insert(m_controlPoints.begin() + index, point);
    }
}

void BezierCurve::removeControlPoint(size_t index) {
    if (index < m_controlPoints.size()) {
        m_controlPoints.erase(m_controlPoints.begin() + index);
    }
}

void BezierCurve::setControlPoints(const std::vector<Point>& points) {
    m_controlPoints = points;
}

void BezierCurve::moveControlPoint(size_t index, const Point& newPosition) {
    if (index < m_controlPoints.size()) {
        m_controlPoints[index] = newPosition;
    }
}

const std::vector<Point>& BezierCurve::getControlPoints() const {
    return m_controlPoints;
}

size_t BezierCurve::getControlPointCount() const {
    return m_controlPoints.size();
}

int BezierCurve::getDegree() const {
    return static_cast<int>(m_controlPoints.size()) - 1;
}

Point BezierCurve::deCasteljau(double t) const {
    if (m_controlPoints.empty()) {
        return Point(0, 0, 0);
    }

    if (m_controlPoints.size() == 1) {
        return m_controlPoints[0];
    }

    // Kopiujemy punkty kontrolne do tymczasowej tablicy
    std::vector<Point> temp = m_controlPoints;

    // Algorytm de Casteljau - iteracyjna implementacja
    for (size_t level = 1; level < m_controlPoints.size(); ++level) {
        for (size_t i = 0; i < m_controlPoints.size() - level; ++i) {
            double x = (1.0 - t) * temp[i].x() + t * temp[i + 1].x();
            double y = (1.0 - t) * temp[i].y() + t * temp[i + 1].y();
            double z = (1.0 - t) * temp[i].z() + t * temp[i + 1].z();
            temp[i] = Point(x, y, z);
        }
    }

    return temp[0];
}

Vector BezierCurve::deCasteljauDerivative(double t) const {
    if (m_controlPoints.size() < 2) {
        return Vector::Zero();
    }

    // Tworzymy krzywą różnicową (derivative curve)
    std::vector<Point> derivativePoints;
    int n = static_cast<int>(m_controlPoints.size()) - 1;

    for (size_t i = 0; i < m_controlPoints.size() - 1; ++i) {
        double dx = n * (m_controlPoints[i + 1].x() - m_controlPoints[i].x());
        double dy = n * (m_controlPoints[i + 1].y() - m_controlPoints[i].y());
        double dz = n * (m_controlPoints[i + 1].z() - m_controlPoints[i].z());
        derivativePoints.emplace_back(dx, dy, dz);
    }

    // Algorytm de Casteljau dla krzywej pochodnej
    for (size_t level = 1; level < derivativePoints.size(); ++level) {
        for (size_t i = 0; i < derivativePoints.size() - level; ++i) {
            double x = (1.0 - t) * derivativePoints[i].x() + t * derivativePoints[i + 1].x();
            double y = (1.0 - t) * derivativePoints[i].y() + t * derivativePoints[i + 1].y();
            double z = (1.0 - t) * derivativePoints[i].z() + t * derivativePoints[i + 1].z();
            derivativePoints[i] = Point(x, y, z);
        }
    }

    if (derivativePoints.empty()) {
        return Vector::Zero();
    }

    return Vector(derivativePoints[0].x(), derivativePoints[0].y(), derivativePoints[0].z());
}

Point BezierCurve::getPointAt(double t) const {
    t = std::max(0.0, std::min(1.0, t));
    return deCasteljau(t);
}

Vector BezierCurve::getTangentAt(double t) const {
    t = std::max(0.0, std::min(1.0, t));
    return deCasteljauDerivative(t);
}

double BezierCurve::getLength() const {
    if (m_controlPoints.size() < 2) {
        return 0.0;
    }

    // Przybliżenie długości przez próbkowanie
    constexpr int numSamples = 1000;
    double length = 0.0;
    Point prevPoint = getPointAt(0.0);

    for (int i = 1; i <= numSamples; ++i) {
        double t = static_cast<double>(i) / numSamples;
        Point currentPoint = getPointAt(t);
        length += prevPoint.distanceTo(currentPoint);
        prevPoint = currentPoint;
    }

    return length;
}

Vector BezierCurve::getPosition() const {
    if (m_controlPoints.empty())
        return Vector::Zero();


    Vector sum = Vector::Zero();
    for (const auto& point : m_controlPoints)
        sum += Vector(point.x(), point.y(), point.z());

    return sum / static_cast<double>(m_controlPoints.size());
}

void BezierCurve::setPosition(const Eigen::Vector3d& position) {
    if (m_controlPoints.empty())
        return;

    const Vector currentCenter = getPosition();
    Vector delta = position - currentCenter;

    for (auto& point : m_controlPoints) {
        point.setX(point.x() + delta.x());
        point.setY(point.y() + delta.y());
        point.setZ(point.z() + delta.z());
    }
}

void BezierCurve::transform(const TransformMatrix& matrix) {
    for (auto& point : m_controlPoints)
        point.transform(matrix);
}

std::unique_ptr<Curve> BezierCurve::clone() const {
    auto copy = std::make_unique<BezierCurve>();
    copy->setControlPoints(m_controlPoints);
    copy->setColor(m_color);
    copy->setLineWidth(m_lineWidth);
    copy->setLineStyle(m_lineStyle);
    copy->setShowControlPolygon(m_showControlPolygon);
    copy->setControlPolygonColor(m_controlPolygonColor);
    copy->setControlPolygonWidth(m_controlPolygonWidth);
    copy->setControlPolygonStyle(m_controlPolygonStyle);
    return copy;
}
