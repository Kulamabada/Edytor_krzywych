#pragma once

#include "../Base/GeometricObject.h"
#include <array>

class TransformMatrix;

class Point : public GeometricObject {
public:
    Point();
    Point(double x, double y, double z = 0.0);
    virtual ~Point() override = default;

    // Coordinate access
    double x() const;
    double y() const;
    double z() const;
    void setX(double x);
    void setY(double y);
    void setZ(double z);

    // Vector operations
    double distanceTo(const Point& other) const;
    Point midPoint(const Point& other) const;

    // Override from GeometricObject
    void transform(const TransformMatrix& matrix) override;

private:
    std::array<double, 3> mCoordinates;
};
