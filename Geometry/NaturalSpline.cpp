#include "NaturalSpline.h"
#include <algorithm>
#include <stdexcept>

NaturalSpline::NaturalSpline(const std::vector<Point2D>& points) : m_points(points) {
    if (points.size() < 2) {
        throw std::invalid_argument("Potrzebne są co najmniej 2 punkty do utworzenia spline'a");
    }
    calculateCoefficients();
}

void NaturalSpline::setPoints(const std::vector<Point2D>& points) {
    if (points.size() < 2) {
        throw std::invalid_argument("Potrzebne są co najmniej 2 punkty do utworzenia spline'a");
    }
    m_points = points;
    calculateCoefficients();
}

const std::vector<NaturalSpline::Point2D>& NaturalSpline::getPoints() const {
    return m_points;
}

std::vector<double> NaturalSpline::calculateDifferences(
    const std::vector<double>& values,
    const std::vector<double>& knots) const {

    const size_t n = values.size() - 1;
    std::vector<double> result;

    for (size_t i = 1; i < n; ++i) {
        double forward = (values[i+1] - values[i]) / (knots[i+1] - knots[i]);
        double backward = (values[i] - values[i-1]) / (knots[i] - knots[i-1]);
        double diff = (forward - backward) / (knots[i+1] - knots[i-1]);
        result.push_back(6.0 * diff);
    }

    return result;
}

std::vector<double> NaturalSpline::calculatePCoefficients(const std::vector<double>& knots) const {
    const size_t n = knots.size() - 1;
    std::vector<double> result = {0.0};

    for (size_t i = 1; i < n - 1; ++i) {
        double lambda = (knots[i] - knots[i-1]) / (knots[i+1] - knots[i-1]);
        double p = (lambda - 1.0) / (lambda * result[i-1] + 2.0);
        result.push_back(p);
    }

    return result;
}

std::vector<double> NaturalSpline::calculateQCoefficients(
    const std::vector<double>& knots,
    const std::vector<double>& differences,
    const std::vector<double>& pCoeffs) const {

    const size_t n = knots.size() - 1;
    std::vector<double> result = {0.0};

    for (size_t i = 1; i < n; ++i) {
        double lambda = (knots[i] - knots[i-1]) / (knots[i+1] - knots[i-1]);
        double q = (differences[i-1] - lambda * result[i-1]) / (lambda * pCoeffs[i-1] + 2.0);
        result.push_back(q);
    }

    return result;
}

std::vector<double> NaturalSpline::calculateMoments(
    const std::vector<double>& pCoeffs,
    const std::vector<double>& qCoeffs) const {

    const size_t n = m_points.size() - 1;
    std::vector<double> moments = {0.0};

    moments.push_back(qCoeffs.back());

    for (size_t i = n - 2; i > 0; --i)
        moments.push_back(pCoeffs[i] * moments.back() + qCoeffs[i]);

    moments.push_back(0.0);

    std::reverse(moments.begin(), moments.end());
    return moments;
}

void NaturalSpline::calculateCoefficients() {
    m_coeffsX.clear();
    m_coeffsY.clear();

    const size_t n = m_points.size();
    if (n < 2) return;

    std::vector<double> knots(n);
    for (size_t i = 0; i < n; ++i) {
        knots[i] = static_cast<double>(i) / (n - 1);
    }

    std::vector<double> xValues(n), yValues(n);
    for (size_t i = 0; i < n; ++i) {
        xValues[i] = m_points[i].first;
        yValues[i] = m_points[i].second;
    }

    std::vector<double> pCoeffs = calculatePCoefficients(knots);
    std::vector<double> dxDiffs = calculateDifferences(xValues, knots);
    std::vector<double> qxCoeffs = calculateQCoefficients(knots, dxDiffs, pCoeffs);
    std::vector<double> xMoments = calculateMoments(pCoeffs, qxCoeffs);

    std::vector<double> dyDiffs = calculateDifferences(yValues, knots);
    std::vector<double> qyCoeffs = calculateQCoefficients(knots, dyDiffs, pCoeffs);
    std::vector<double> yMoments = calculateMoments(pCoeffs, qyCoeffs);

    for (size_t i = 0; i < n - 1; ++i) {
        double h = knots[i+1] - knots[i];

        std::vector<double> xCoeffs(4);
        xCoeffs[0] = xValues[i];
        xCoeffs[1] = xMoments[i] / 2.0;
        xCoeffs[2] = (xValues[i+1] - xValues[i]) / h - h * (2.0 * xMoments[i] + xMoments[i+1]) / 6.0;
        xCoeffs[3] = (xMoments[i+1] - xMoments[i]) / (6.0 * h);

        std::vector<double> yCoeffs(4);
        yCoeffs[0] = yValues[i];
        yCoeffs[1] = yMoments[i] / 2.0;
        yCoeffs[2] = (yValues[i+1] - yValues[i]) / h - h * (2.0 * yMoments[i] + yMoments[i+1]) / 6.0;
        yCoeffs[3] = (yMoments[i+1] - yMoments[i]) / (6.0 * h);

        m_coeffsX.push_back(xCoeffs);
        m_coeffsY.push_back(yCoeffs);
    }
}

NaturalSpline::Point2D NaturalSpline::evaluate(double t) const {
    if (m_points.empty()) {
        return {0.0, 0.0};
    }

    if (m_points.size() == 1) {
        return m_points[0];
    }

    t = std::max(0.0, std::min(1.0, t));

    const size_t n = m_points.size();
    const double scaledT = t * (n - 1);

    size_t segment = static_cast<size_t>(scaledT);
    if (segment >= n - 1) {
        segment = n - 2;
    }

    double u = scaledT - segment;

    const auto& xCoeffs = m_coeffsX[segment];
    const auto& yCoeffs = m_coeffsY[segment];

    double x = xCoeffs[0] + u * (xCoeffs[1] + u * (xCoeffs[2] + u * xCoeffs[3]));
    double y = yCoeffs[0] + u * (yCoeffs[1] + u * (yCoeffs[2] + u * yCoeffs[3]));

    return {x, y};
}
