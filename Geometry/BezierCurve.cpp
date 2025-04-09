#include "BezierCurve.h"
#include "Point.h"
#include "Vector.h"
#include "IMathProvider.h"
#include <Eigen/Dense>
#include <cmath>

BezierCurve::BezierCurve(std::shared_ptr<IMathProvider> mathProvider)
    : m_mathProvider(std::move(mathProvider)) {
    // Inicjalizacja krzywej Beziera
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

Point BezierCurve::getPointAt(double t) const {
    if (m_controlPoints.empty()) {
        return Point(0.0, 0.0);
    }

    // Zastosowanie algorytmu de Casteljau do obliczenia punktu na krzywej
    return evaluateDeCasteljau(m_controlPoints, t);
}

Vector BezierCurve::getTangentAt(double t) const {
    if (m_controlPoints.size() < 2) {
        return Vector(0.0, 0.0);
    }

    // Dla krzywej Beziera stopnia n, pochodna to krzywa Beziera stopnia n-1
    // z punktami kontrolnymi będącymi różnicami sąsiednich punktów początkowej krzywej
    std::vector<Point> derivativeControlPoints;
    for (size_t i = 0; i < m_controlPoints.size() - 1; ++i) {
        double dx = m_controlPoints[i + 1].x() - m_controlPoints[i].x();
        double dy = m_controlPoints[i + 1].y() - m_controlPoints[i].y();
        double dz = m_controlPoints[i + 1].z() - m_controlPoints[i].z();

        // Mnożymy przez stopień krzywej
        dx *= (m_controlPoints.size() - 1);
        dy *= (m_controlPoints.size() - 1);
        dz *= (m_controlPoints.size() - 1);

        derivativeControlPoints.emplace_back(dx, dy, dz);
    }

    // Obliczamy punkt na krzywej pochodnej
    Point derivativePoint = evaluateDeCasteljau(derivativeControlPoints, t);

    return Vector(derivativePoint.x(), derivativePoint.y(), derivativePoint.z());
}

double BezierCurve::getCurvatureAt(double t) const {
    if (m_controlPoints.size() < 3) {
        return 0.0; // Krzywa liniowa ma zerową krzywiznę
    }

    // Obliczamy pierwszą pochodną
    Vector firstDerivative = getTangentAt(t);

    // Obliczamy drugą pochodną (pochodna z pochodnej)
    // Najpierw tworzymy punkty kontrolne dla pierwszej pochodnej
    std::vector<Point> firstDerivativeControlPoints;
    for (size_t i = 0; i < m_controlPoints.size() - 1; ++i) {
        double dx = m_controlPoints[i + 1].x() - m_controlPoints[i].x();
        double dy = m_controlPoints[i + 1].y() - m_controlPoints[i].y();
        double dz = m_controlPoints[i + 1].z() - m_controlPoints[i].z();

        // Mnożymy przez stopień krzywej
        dx *= (m_controlPoints.size() - 1);
        dy *= (m_controlPoints.size() - 1);
        dz *= (m_controlPoints.size() - 1);

        firstDerivativeControlPoints.emplace_back(dx, dy, dz);
    }

    // Teraz tworzymy punkty kontrolne dla drugiej pochodnej
    std::vector<Point> secondDerivativeControlPoints;
    for (size_t i = 0; i < firstDerivativeControlPoints.size() - 1; ++i) {
        double dx = firstDerivativeControlPoints[i + 1].x() - firstDerivativeControlPoints[i].x();
        double dy = firstDerivativeControlPoints[i + 1].y() - firstDerivativeControlPoints[i].y();
        double dz = firstDerivativeControlPoints[i + 1].z() - firstDerivativeControlPoints[i].z();

        // Mnożymy przez stopień krzywej pochodnej
        dx *= (firstDerivativeControlPoints.size() - 1);
        dy *= (firstDerivativeControlPoints.size() - 1);
        dz *= (firstDerivativeControlPoints.size() - 1);

        secondDerivativeControlPoints.emplace_back(dx, dy, dz);
    }

    Point secondDerivativePoint = evaluateDeCasteljau(secondDerivativeControlPoints, t);
    Vector secondDerivative(secondDerivativePoint.x(), secondDerivativePoint.y(), secondDerivativePoint.z());

    // Obliczamy krzywiznę w przestrzeni 2D
    // Formuła: κ = |x'y'' - y'x''| / (x'^2 + y'^2)^(3/2)
    double numerator = std::abs(firstDerivative.x() * secondDerivative.y() -
                               firstDerivative.y() * secondDerivative.x());
    double denominator = std::pow(firstDerivative.x() * firstDerivative.x() +
                                 firstDerivative.y() * firstDerivative.y(), 1.5);

    if (denominator < 1e-10) {
        return 0.0; // Unikamy dzielenia przez zero
    }

    return numerator / denominator;
}

Point BezierCurve::evaluateDeCasteljau(const std::vector<Point>& points, double t) const {
    // Warunek zakończenia rekurencji
    if (points.size() == 1) {
        return points[0];
    }

    // Implementacja algorytmu de Casteljau
    std::vector<Point> reducedPoints;
    for (size_t i = 0; i < points.size() - 1; ++i) {
        double x = (1.0 - t) * points[i].x() + t * points[i + 1].x();
        double y = (1.0 - t) * points[i].y() + t * points[i + 1].y();
        double z = (1.0 - t) * points[i].z() + t * points[i + 1].z();
        reducedPoints.emplace_back(x, y, z);
    }

    // Rekurencyjne wywołanie dla zredukowanego zbioru punktów
    return evaluateDeCasteljau(reducedPoints, t);
}
