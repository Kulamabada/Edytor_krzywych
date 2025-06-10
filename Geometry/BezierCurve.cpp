#include "BezierCurve.h"
#include <stdexcept>
#include <algorithm>
#include <numeric>

BezierCurve::BezierCurve() : Curve(CurveType::Bezier) {
}

int BezierCurve::getDegree() const {
    return static_cast<int>(m_controlPoints.size()) - 1;
}

std::unique_ptr<Curve> BezierCurve::createClone() const {
    auto clone = std::make_unique<BezierCurve>();
    return clone;
}

void BezierCurve::copySpecificPropertiesTo(Curve* target) const {
    if (auto bezierTarget = dynamic_cast<BezierCurve*>(target)) {
        bezierTarget->m_weights = m_weights;
        bezierTarget->m_showWeights = m_showWeights;
    }
}

Point BezierCurve::getPointAt(double t) const {
    validateParameterRange(t);

    if (hasWeights() && !std::ranges::all_of(m_weights, [this](double w){ return std::abs(w - m_weights[0]) < 1e-9; }))
        return rationalDeCasteljau(t);
    return deCasteljau(t);
}

void BezierCurve::setWeights(const std::vector<double>& weights) {
    if (weights.size() != m_controlPoints.size())
        throw std::invalid_argument("Liczba wag musi być równa liczbie punktów kontrolnych");

    if (std::any_of(weights.begin(), weights.end(), [](double w) { return w <= 0.0; }))
        throw std::invalid_argument("Wszystkie wagi muszą być dodatnie");

    m_weights = weights;
    onControlPointsChanged();
}

const std::vector<double>& BezierCurve::getWeights() const {
    return m_weights;
}

void BezierCurve::setWeight(size_t index, double weight) {
    if (index >= m_controlPoints.size())
        throw std::out_of_range("Indeks poza zakresem");

    if (weight <= 0.0)
        throw std::invalid_argument("Waga musi być dodatnia");

    ensureWeightsSize();
    m_weights[index] = weight;
    onControlPointsChanged();
}

double BezierCurve::getWeight(size_t index) const {
    if (index >= m_controlPoints.size())
        throw std::out_of_range("Indeks poza zakresem");

    if (hasWeights() && index < m_weights.size())
        return m_weights[index];

    return 1.0;
}

bool BezierCurve::hasWeights() const {
    return !m_weights.empty() && m_weights.size() == m_controlPoints.size();
}

void BezierCurve::resetWeights() {
    if (m_weights.empty())
        ensureWeightsSize();

    std::ranges::fill(m_weights, 1.0);
    onControlPointsChanged();
}

void BezierCurve::normalizeWeights() {
    if (m_weights.empty()) return;

    if (const double sum = std::accumulate(m_weights.begin(), m_weights.end(), 0.0); sum > 0.0) {
        const double factor = static_cast<double>(m_weights.size()) / sum;
        for (double& weight : m_weights)
            weight *= factor;

        onControlPointsChanged();
    }
}

void BezierCurve::addControlPoint(const Point& point) {
    Curve::addControlPoint(point);

    if (!m_weights.empty()) m_weights.push_back(1.0);
}

void BezierCurve::insertControlPoint(const size_t index, const Point& point) {
    if (index <= m_controlPoints.size()) {
        Curve::insertControlPoint(index, point);

        if (!m_weights.empty() && index < m_weights.size())
            m_weights.insert(m_weights.begin() + index, 1.0);
        else if (!m_weights.empty())
            m_weights.push_back(1.0);
    }
}

void BezierCurve::removeControlPoint(size_t index) {
    if (index < m_controlPoints.size()) {
        Curve::removeControlPoint(index);

        if (!m_weights.empty() && index < m_weights.size())
            m_weights.erase(m_weights.begin() + index);
    }
}

void BezierCurve::setControlPoints(const std::vector<Point>& points) {
    Curve::setControlPoints(points);

    if (!m_weights.empty())
        m_weights.resize(points.size(), 1.0);
}

Point BezierCurve::deCasteljau(const double t) const {
    if (m_controlPoints.empty())
        return Point(0, 0, 0);

    if (m_controlPoints.size() == 1)
        return m_controlPoints[0];

    std::vector<Point> temp = m_controlPoints;

    for (size_t level = 1; level < m_controlPoints.size(); ++level)
        for (size_t i = 0; i < m_controlPoints.size() - level; ++i)
            temp[i] = interpolateLinear(temp[i], temp[i + 1], t);

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
            const double newWeight = (1.0 - t) * tempWeights[i] + t * tempWeights[i + 1];

            const double w1 = (1.0 - t) * tempWeights[i];
            const double w2 = t * tempWeights[i + 1];

            const Point& p1 = tempPoints[i];
            const Point& p2 = tempPoints[i + 1];

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
    const int n = static_cast<int>(m_controlPoints.size()) - 1;

    for (size_t i = 0; i < m_controlPoints.size() - 1; ++i) {
        double dx = n * (m_controlPoints[i + 1].x() - m_controlPoints[i].x());
        double dy = n * (m_controlPoints[i + 1].y() - m_controlPoints[i].y());
        double dz = n * (m_controlPoints[i + 1].z() - m_controlPoints[i].z());
        derivativePoints.emplace_back(dx, dy, dz);
    }

    for (size_t level = 1; level < derivativePoints.size(); ++level)
        for (size_t i = 0; i < derivativePoints.size() - level; ++i)
            derivativePoints[i] = interpolateLinear(derivativePoints[i], derivativePoints[i + 1], t);

    if (derivativePoints.empty())
        return Vector::Zero();

    return Vector(derivativePoints[0].x(), derivativePoints[0].y(), derivativePoints[0].z());
}

void BezierCurve::ensureWeightsSize() {
    if (m_weights.size() != m_controlPoints.size())
        m_weights.resize(m_controlPoints.size(), 1.0);
}

void BezierCurve::validateWeights() const {
    if (m_weights.size() != m_controlPoints.size())
        throw std::runtime_error("Rozmiar wektorów wag i punktów kontrolnych nie jest zgodny");

    if (std::ranges::any_of(m_weights, [](double w) { return w <= 0.0; }))
        throw std::runtime_error("Wszystkie wagi muszą być dodatnie");
}

void BezierCurve::setShowWeights(bool show) {
    m_showWeights = show;

    if (show && !hasWeights() && !m_controlPoints.empty()) {
        ensureWeightsSize();
        resetWeights();
    }
}

std::vector<std::vector<Point>> BezierCurve::getDeCasteljauSteps() const {
    std::vector<std::vector<Point>> steps;
    if (m_controlPoints.size() < 2) return steps;

    if (hasWeights() && !std::ranges::all_of(m_weights, [this](double w){ return std::abs(w - m_weights[0]) < 1e-9; })) {
        const size_t n = m_controlPoints.size();
        std::vector<Point> tempPoints = m_controlPoints;
        std::vector<double> tempWeights = m_weights;

        steps.push_back(tempPoints);

        for (size_t level = 1; level < n; ++level) {
            std::vector<Point> nextPoints;
            std::vector<double> nextWeights;

            for (size_t i = 0; i < n - level; ++i) {
                const double newWeight = (1.0 - m_deCasteljauT) * tempWeights[i] + m_deCasteljauT * tempWeights[i + 1];
                const double w1 = (1.0 - m_deCasteljauT) * tempWeights[i];
                const double w2 = m_deCasteljauT * tempWeights[i + 1];

                const Point& p1 = tempPoints[i];
                const Point& p2 = tempPoints[i + 1];

                const double invNewWeight = 1.0 / newWeight;
                Point newPoint(
                    (w1 * p1.x() + w2 * p2.x()) * invNewWeight,
                    (w1 * p1.y() + w2 * p2.y()) * invNewWeight,
                    (w1 * p1.z() + w2 * p2.z()) * invNewWeight
                );

                nextPoints.push_back(newPoint);
                nextWeights.push_back(newWeight);
            }

            steps.push_back(nextPoints);
            tempPoints = nextPoints;
            tempWeights = nextWeights;
        }
    } else {
        std::vector<Point> currentLevel = m_controlPoints;
        steps.push_back(currentLevel);

        for (size_t level = 1; level < m_controlPoints.size(); ++level) {
            std::vector<Point> nextLevel;
            for (size_t i = 0; i < currentLevel.size() - 1; ++i) {
                Point interpolated = interpolateLinear(currentLevel[i], currentLevel[i + 1], m_deCasteljauT);
                nextLevel.push_back(interpolated);
            }
            steps.push_back(nextLevel);
            currentLevel = nextLevel;
        }
    }

    return steps;
}

QJsonObject BezierCurve::toJson() const {
    QJsonObject json = serializeCurveData();

    if (hasWeights()) {
        QJsonArray weightsArray;
        for (double weight : m_weights) {
            weightsArray.append(weight);
        }
        json["weights"] = weightsArray;
    }

    json["showWeights"] = m_showWeights;
    json["showDeCasteljau"] = m_showDeCasteljau;
    json["deCasteljauT"] = m_deCasteljauT;

    return json;
}

bool BezierCurve::fromJson(const QJsonObject& json) {
    if (!deserializeCurveData(json)) return false;

    if (json.contains("weights")) {
        QJsonArray weightsArray = json["weights"].toArray();
        std::vector<double> weights;

        for (const auto& value : weightsArray) {
            weights.push_back(value.toDouble(1.0));
        }

        if (!weights.empty()) {
            setWeights(weights);
        }
    }

    m_showWeights = json["showWeights"].toBool(false);
    m_showDeCasteljau = json["showDeCasteljau"].toBool(false);
    m_deCasteljauT = json["deCasteljauT"].toDouble(0.5);

    return true;
}