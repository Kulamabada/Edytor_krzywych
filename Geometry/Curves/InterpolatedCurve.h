#pragma once

#include "Curve.h"
#include <vector>
#include <memory>

class IMathProvider;

class InterpolatedCurve : public Curve {
public:
    enum class InterpolationType {
        Linear,         // Interpolacja liniowa
        CubicSpline,    // Naturalne spline'y kubiczne
        CatmullRom,     // Interpolacja Catmull-Rom
        CardinalSpline  // Spline kardynalne z parametrem napięcia
    };

    explicit InterpolatedCurve(std::shared_ptr<IMathProvider> mathProvider, 
                             InterpolationType type = InterpolationType::CubicSpline);
    ~InterpolatedCurve() override = default;

    // Zarządzanie punktami interpolacji
    void addPoint(const Point& point);
    void insertPoint(size_t index, const Point& point);
    void removePoint(size_t index);
    void setPoints(const std::vector<Point>& points);
    void movePoint(size_t index, const Point& newPosition);
    const std::vector<Point>& getPoints() const;
    size_t getPointCount() const;

    // Konfiguracja interpolacji
    void setInterpolationType(InterpolationType type);
    InterpolationType getInterpolationType() const;
    
    // Parametry dla określonych typów interpolacji
    void setTension(double tension); // Dla Cardinal splines
    double getTension() const;

    // Przebudowa danych interpolacji
    void rebuildInterpolation();

    // Implementacja metod z klasy bazowej Curve
    Point getPointAt(double t) const override;
    Vector getTangentAt(double t) const override;
    double getCurvatureAt(double t) const override;

private:
    std::vector<Point> points;
    InterpolationType type;
    double tension = 0.5; // Domyślna wartość napięcia dla Cardinal splines
    bool needsRebuild = true;
    std::shared_ptr<IMathProvider> mathProvider;
    
    // Wewnętrzne dane interpolacji
    class InterpolationData;
    std::unique_ptr<InterpolationData> data;
};
