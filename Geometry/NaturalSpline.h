#pragma once

#include <vector>
#include <utility>
#include <cmath>

class NaturalSpline {
public:
    using Point2D = std::pair<double, double>;

    explicit NaturalSpline(const std::vector<Point2D>& points);

    //t ∈ [0,1]
    Point2D evaluate(double t) const;

    void setPoints(const std::vector<Point2D>& points);

    const std::vector<Point2D>& getPoints() const;

private:
    std::vector<Point2D> m_points;

    std::vector<std::vector<double>> m_coeffsX;
    std::vector<std::vector<double>> m_coeffsY;

    std::vector<double> calculateDifferences(const std::vector<double>& values,
                                           const std::vector<double>& knots) const;

    std::vector<double> calculatePCoefficients(const std::vector<double>& knots) const;

    std::vector<double> calculateQCoefficients(const std::vector<double>& knots,
                                             const std::vector<double>& differences,
                                             const std::vector<double>& pCoeffs) const;

    std::vector<double> calculateMoments(const std::vector<double>& pCoeffs,
                                        const std::vector<double>& qCoeffs) const;

    void calculateCoefficients();
};
