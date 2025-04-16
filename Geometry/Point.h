#pragma once

#include "GeometricObject.h"
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
    Eigen::Vector3d getPosition() const override;
    void setPosition(const Eigen::Vector3d& position) override;

private:
    std::array<double, 3> m_coordinates;
};
