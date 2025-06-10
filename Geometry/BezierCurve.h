#pragma once

#include "Curve.h"

class BezierCurve final : public Curve {
public:
    explicit BezierCurve();
    ~BezierCurve() override = default;

    int getDegree() const;

    Point getPointAt(double t) const override;

    void setWeights(const std::vector<double>& weights);
    const std::vector<double>& getWeights() const;
    void setWeight(size_t index, double weight);
    double getWeight(size_t index) const;
    bool hasWeights() const;

    void resetWeights();
    void normalizeWeights();

    void addControlPoint(const Point& point) override;
    void insertControlPoint(size_t index, const Point& point) override;
    void removeControlPoint(size_t index) override;
    void setControlPoints(const std::vector<Point>& points) override;

    void setShowWeights(bool show);
    bool getShowWeights() const { return m_showWeights; }

    void setShowDeCasteljau(bool show) { m_showDeCasteljau = show; }
    bool getShowDeCasteljau() const { return m_showDeCasteljau; }

    void setDeCasteljauT(double t) { m_deCasteljauT = std::clamp(t, 0.0, 1.0); }
    double getDeCasteljauT() const { return m_deCasteljauT; }

    std::vector<std::vector<Point>> getDeCasteljauSteps() const;

protected:
    std::unique_ptr<Curve> createClone() const override;
    void copySpecificPropertiesTo(Curve* target) const override;

private:
    Point deCasteljau(double t) const;
    Point rationalDeCasteljau(double t) const;
    Vector deCasteljauDerivative(double t) const;

    void ensureWeightsSize();
    void validateWeights() const;

    std::vector<double> m_weights;
    bool m_showWeights{false};

    bool m_showDeCasteljau{false};
    double m_deCasteljauT{0.5};
};
