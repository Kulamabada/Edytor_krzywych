#pragma once

#include "GeometricObject.h"
#include <vector>
#include <memory>

// Forward declarations
class Point;

class Curve : public GeometricObject {
public:
    Curve();
    virtual ~Curve() override = default;

    // Curve-specific methods
    virtual bool isPointOnCurve(const Point& point, double tolerance = 1e-6) const = 0;
    virtual double getLength() const = 0;
    virtual Point getPointAt(double t) const = 0;
    virtual Vector getTangentAt(double t) const = 0;
    virtual double getCurvatureAt(double t) const = 0;

    virtual std::vector<Point> getIntersectionsWith(const Curve& other) const = 0;

    // Override from GeometricObject
    void transform(const TransformMatrix& matrix) override = 0;

    // Common operations for all curves
    virtual void reverse();
    virtual std::unique_ptr<Curve> clone() const = 0;
};
