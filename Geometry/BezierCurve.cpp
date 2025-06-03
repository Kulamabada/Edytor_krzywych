#include "BezierCurve.h"
#include <stdexcept>
#include <algorithm>

BezierCurve::BezierCurve() : Curve(CurveType::Bezier) {
}

int BezierCurve::getDegree() const {
    return static_cast<int>(m_controlPoints.size()) - 1;
}

std::unique_ptr<Curve> BezierCurve::createClone() const {
    return std::make_unique<BezierCurve>();
}

void BezierCurve::copySpecificPropertiesTo(Curve* target) const {
    if (auto bezierTarget = dynamic_cast<BezierCurve*>(target)) {
        bezierTarget->m_weights = m_weights;
    }
}

Point BezierCurve::getPointAt(double t) const {
    validateParameterRange(t);

    if (hasWeights()) {
        return rationalDeCasteljau(t);
    } else {
        return deCasteljau(t);
    }
}

Vector BezierCurve::getTangentAt(double t) const {
    validateParameterRange(t);

    if (hasWeights()) {
        return rationalDeCasteljauDerivative(t);
    } else {
        return deCasteljauDerivative(t);
    }
}

// === Obsługa wag ===

void BezierCurve::setWeights(const std::vector<double>& weights) {
    if (weights.size() != m_controlPoints.size()) {
        throw std::invalid_argument("Liczba wag musi być równa liczbie punktów kontrolnych");
    }

    // Sprawdzenie czy wszystkie wagi są dodatnie
    if (std::any_of(weights.begin(), weights.end(), [](double w) { return w <= 0.0; })) {
        throw std::invalid_argument("Wszystkie wagi muszą być dodatnie");
    }

    m_weights = weights;
    onControlPointsChanged();
}

const std::vector<double>& BezierCurve::getWeights() const {
    return m_weights;
}

void BezierCurve::setWeight(size_t index, double weight) {
    if (index >= m_controlPoints.size()) {
        throw std::out_of_range("Indeks poza zakresem");
    }

    if (weight <= 0.0) {
        throw std::invalid_argument("Waga musi być dodatnia");
    }

    ensureWeightsSize();
    m_weights[index] = weight;
    onControlPointsChanged();
}

double BezierCurve::getWeight(size_t index) const {
    if (index >= m_controlPoints.size()) {
        throw std::out_of_range("Indeks poza zakresem");
    }

    if (hasWeights() && index < m_weights.size()) {
        return m_weights[index];
    }

    return 1.0; // Domyślna waga
}

bool BezierCurve::hasWeights() const {
    return !m_weights.empty() && m_weights.size() == m_controlPoints.size();
}

void BezierCurve::clearWeights() {
    m_weights.clear();
    onControlPointsChanged();
}

// === Nadpisane metody z Curve ===

void BezierCurve::addControlPoint(const Point& point) {
    Curve::addControlPoint(point);

    if (hasWeights()) {
        m_weights.push_back(1.0); // Domyślna waga dla nowego punktu
    }
}

void BezierCurve::insertControlPoint(size_t index, const Point& point) {
    if (index <= m_controlPoints.size()) {
        Curve::insertControlPoint(index, point);

        if (hasWeights()) {
            m_weights.insert(m_weights.begin() + index, 1.0);
        }
    }
}

void BezierCurve::removeControlPoint(size_t index) {
    if (index < m_controlPoints.size()) {
        Curve::removeControlPoint(index);

        if (hasWeights() && index < m_weights.size()) {
            m_weights.erase(m_weights.begin() + index);
        }
    }
}

void BezierCurve::setControlPoints(const std::vector<Point>& points) {
    Curve::setControlPoints(points);

    // Jeśli wcześniej były wagi, dostosuj ich liczbę
    if (!m_weights.empty()) {
        m_weights.resize(points.size(), 1.0);
    }
}

// === Algorytmy de Casteljau ===

Point BezierCurve::deCasteljau(const double t) const {
    if (m_controlPoints.empty())
        return Point(0, 0, 0);

    if (m_controlPoints.size() == 1)
        return m_controlPoints[0];

    std::vector<Point> temp = m_controlPoints;

    for (size_t level = 1; level < m_controlPoints.size(); ++level) {
        for (size_t i = 0; i < m_controlPoints.size() - level; ++i) {
            temp[i] = interpolateLinear(temp[i], temp[i + 1], t);
        }
    }

    return temp[0];
}

Point BezierCurve::rationalDeCasteljau(double t) const {
    if (m_controlPoints.empty())
        return Point(0, 0, 0);

    if (m_controlPoints.size() == 1)
        return m_controlPoints[0];

    validateWeights();

    const size_t n = m_controlPoints.size();
    std::vector<Point> tempPoints = m_controlPoints;
    std::vector<double> tempWeights = m_weights;

    for (size_t level = 1; level < n; ++level) {
        for (size_t i = 0; i < n - level; ++i) {
            // Oblicz nową wagę
            const double newWeight = (1.0 - t) * tempWeights[i] + t * tempWeights[i + 1];

            // Oblicz nowy punkt używając wag
            const double w1 = (1.0 - t) * tempWeights[i];
            const double w2 = t * tempWeights[i + 1];

            const Point& p1 = tempPoints[i];
            const Point& p2 = tempPoints[i + 1];

            // Interpolacja z wagami: (w1*P1 + w2*P2) / (w1 + w2)
            const double invNewWeight = 1.0 / newWeight;
            tempPoints[i] = Point(
                (w1 * p1.x() + w2 * p2.x()) * invNewWeight,
                (w1 * p1.y() + w2 * p2.y()) * invNewWeight,
                (w1 * p1.z() + w2 * p2.z()) * invNewWeight
            );

            tempWeights[i] = newWeight;
        }
    }

    return tempPoints[0];
}

Vector BezierCurve::deCasteljauDerivative(double t) const {
    if (m_controlPoints.size() < 2)
        return Vector::Zero();

    std::vector<Point> derivativePoints;
    int n = static_cast<int>(m_controlPoints.size()) - 1;

    for (size_t i = 0; i < m_controlPoints.size() - 1; ++i) {
        double dx = n * (m_controlPoints[i + 1].x() - m_controlPoints[i].x());
        double dy = n * (m_controlPoints[i + 1].y() - m_controlPoints[i].y());
        double dz = n * (m_controlPoints[i + 1].z() - m_controlPoints[i].z());
        derivativePoints.emplace_back(dx, dy, dz);
    }

    for (size_t level = 1; level < derivativePoints.size(); ++level)
        for (size_t i = 0; i < derivativePoints.size() - level; ++i) {
            derivativePoints[i] = interpolateLinear(derivativePoints[i], derivativePoints[i + 1], t);
        }

    if (derivativePoints.empty())
        return Vector::Zero();

    return Vector(derivativePoints[0].x(), derivativePoints[0].y(), derivativePoints[0].z());
}

Vector BezierCurve::rationalDeCasteljauDerivative(double t) const {
    if (m_controlPoints.size() < 2)
        return Vector::Zero();

    validateWeights();

    // Dla wymiernej krzywej Béziera: C(t) = S(t) / W(t)
    // Pochodna: C'(t) = (S'(t) * W(t) - S(t) * W'(t)) / W(t)^2

    // Oblicz S(t) - numerator (suma wagowanych punktów)
    Point numerator = rationalDeCasteljau(t);
    double denominator = 0.0;

    // Oblicz W(t) - mianownik (suma wag)
    const size_t n = m_controlPoints.size();
    std::vector<double> tempWeights = m_weights;

    for (size_t level = 1; level < n; ++level) {
        for (size_t i = 0; i < n - level; ++i) {
            tempWeights[i] = (1.0 - t) * tempWeights[i] + t * tempWeights[i + 1];
        }
    }
    denominator = tempWeights[0];

    // Oblicz pochodne używając różnic skończonych dla dokładności
    const double epsilon = 1e-6;
    const double t1 = std::max(0.0, t - epsilon);
    const double t2 = std::min(1.0, t + epsilon);

    const Point p1 = rationalDeCasteljau(t1);
    const Point p2 = rationalDeCasteljau(t2);
    const double dt = t2 - t1;

    return Vector(
        (p2.x() - p1.x()) / dt,
        (p2.y() - p1.y()) / dt,
        (p2.z() - p1.z()) / dt
    );
}

// === Metody pomocnicze ===

void BezierCurve::ensureWeightsSize() {
    if (m_weights.size() != m_controlPoints.size()) {
        m_weights.resize(m_controlPoints.size(), 1.0);
    }
}

void BezierCurve::validateWeights() const {
    if (m_weights.size() != m_controlPoints.size()) {
        throw std::runtime_error("Rozmiar wektorów wag i punktów kontrolnych nie jest zgodny");
    }

    if (std::any_of(m_weights.begin(), m_weights.end(), [](double w) { return w <= 0.0; })) {
        throw std::runtime_error("Wszystkie wagi muszą być dodatnie");
    }
}
