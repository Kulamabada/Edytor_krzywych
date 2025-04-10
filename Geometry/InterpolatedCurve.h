#pragma once

#include "Curve.h"
#include <vector>
#include <memory>
#include <Eigen/Dense>

class IMathProvider;
class Point;

class InterpolatedCurve : public Curve {
public:
    enum class InterpolationType {
        Linear,         // Interpolacja liniowa
        CubicSpline,    // Naturalne spline'y kubiczne
        CatmullRom      // Interpolacja Catmull-Rom
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

    // Implementacja metod z klasy bazowej Curve
    bool isPointOnCurve(const Point& point, double tolerance = 1e-6) const override;
    double getLength() const override;
    Point getPointAt(double t) const override;
    Vector getTangentAt(double t) const override;
    double getCurvatureAt(double t) const override;
    std::vector<Point> getIntersectionsWith(const Curve& other) const override;

    // Implementacja z GeometricObject
    void transform(const TransformMatrix& matrix) override;
    std::unique_ptr<Curve> clone() const override;
    Vector getPosition() const override;
    void setPosition(const Eigen::Vector3d& position) override;
    Vector getScale() const override;
    void setScale(const Vector& scale) override;
    Eigen::Quaterniond getRotation() const override;
    void setRotation(const Eigen::Quaterniond& rotation) override;

private:
    std::vector<Point> points;
    InterpolationType type;
    std::shared_ptr<IMathProvider> mathProvider;

    // Buforowane dane interpolacyjne
    mutable std::vector<Eigen::Matrix<double, 4, 2>> coefficients;
    mutable bool needsRebuild{true};

    // Metody do budowy danych interpolacyjnych
    void rebuildInterpolation() const;
    void buildCubicSpline() const;
    void buildCatmullRom() const;
};
