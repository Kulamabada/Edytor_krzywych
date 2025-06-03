#pragma once

#include "Curve.h"

class BezierCurve final : public Curve {
public:
    explicit BezierCurve();
    ~BezierCurve() override = default;

    int getDegree() const;

    Point getPointAt(double t) const override;
    Vector getTangentAt(double t) const override;

    // Obsługa wag dla wymiernych krzywych Béziera
    void setWeights(const std::vector<double>& weights);
    const std::vector<double>& getWeights() const;
    void setWeight(size_t index, double weight);
    double getWeight(size_t index) const;
    bool hasWeights() const;
    void clearWeights();

    // Nadpisane metody z Curve dla obsługi wag
    void addControlPoint(const Point& point) override;
    void insertControlPoint(size_t index, const Point& point) override;
    void removeControlPoint(size_t index) override;
    void setControlPoints(const std::vector<Point>& points) override;

protected:
    std::unique_ptr<Curve> createClone() const override;
    void copySpecificPropertiesTo(Curve* target) const override;

private:
    // Algorytmy de Casteljau
    Point deCasteljau(double t) const;                    // Zwykły algorytm
    Point rationalDeCasteljau(double t) const;            // Wymierny algorytm
    Vector deCasteljauDerivative(double t) const;         // Zwykła pochodna
    Vector rationalDeCasteljauDerivative(double t) const; // Wymierna pochodna

    // Pomocnicze metody dla wymiernych krzywych
    void ensureWeightsSize();
    void validateWeights() const;

    std::vector<double> m_weights;
};
