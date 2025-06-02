#pragma once

#include "GeometricObject.h"
#include <vector>
#include "Point.h"
#include <QColor>

// Forward declarations
class Point;

class Curve : public GeometricObject {
public:
    Curve(){};
    ~Curve() override = default;

    virtual double getLength() const = 0;
    virtual Point getPointAt(double t) const = 0;
    virtual Vector getTangentAt(double t) const = 0;

    virtual std::unique_ptr<Curve> clone() const = 0;

    virtual bool hasControlPoints() const { return false; }
    virtual void addControlPoint(const Point& point) {}
    virtual void insertControlPoint(size_t index, const Point& point) {}
    virtual void removeControlPoint(size_t index) {}
    virtual void setControlPoints(const std::vector<Point>& points) {}
    virtual void moveControlPoint(size_t index, const Point& newPosition) {}
    virtual const std::vector<Point>& getControlPoints() const {
        static std::vector<Point> empty;
        return empty;
    }
    virtual size_t getControlPointCount() const { return 0; }

    // Override from GeometricObject
    void transform(const TransformMatrix& matrix) override = 0;
};
