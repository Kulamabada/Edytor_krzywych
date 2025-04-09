#pragma once

#include "Curve.h"
#include <vector>
#include <memory>

class IMathProvider;

class BezierCurve : public Curve {
public:
    explicit BezierCurve(std::shared_ptr<IMathProvider> mathProvider);
    ~BezierCurve() override = default;

    // Zarządzanie punktami kontrolnymi
    void addControlPoint(const Point& point);
    void insertControlPoint(size_t index, const Point& point);
    void removeControlPoint(size_t index);
    void setControlPoints(const std::vector<Point>& points);
    void moveControlPoint(size_t index, const Point& newPosition);
    const std::vector<Point>& getControlPoints() const;
    size_t getControlPointCount() const;

    // Implementacja metod z klasy bazowej Curve
    Point getPointAt(double t) const override;
    Vector getTangentAt(double t) const override;
    double getCurvatureAt(double t) const override;

private:
    // Implementacja algorytmu de Casteljau
    Point evaluateDeCasteljau(const std::vector<Point>& points, double t) const;
    
    std::vector<Point> controlPoints;
    std::shared_ptr<IMathProvider> mathProvider;
};
