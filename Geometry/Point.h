#pragma once

#include "GeometricObject.h"
#include "../Core/JsonSerializable.h"
#include <array>
#include "TransformMatrix.h"
#include <cmath>

class TransformMatrix;

class Point : public GeometricObject , public JsonSerializable {
public:
    Point() : m_coordinates{0.0, 0.0, 0.0} {}
    Point(double x, double y, double z = 0.0) : m_coordinates{x, y, z} {}
    virtual ~Point() override = default;

    // Coordinate access - inline implementations
    double x() const { return m_coordinates[0]; }
    double y() const { return m_coordinates[1]; }
    double z() const { return m_coordinates[2]; }
    void setX(double x) { m_coordinates[0] = x; }
    void setY(double y) { m_coordinates[1] = y; }
    void setZ(double z) { m_coordinates[2] = z; }

    // Vector operations
    double distanceTo(const Point& other) const;
    Point midPoint(const Point& other) const;

    // Override from GeometricObject
    void transform(const TransformMatrix& matrix) override;

    // Również inline jako proste metody dostępowe
    Eigen::Vector3d getPosition() const override { return Eigen::Vector3d(x(), y(), z()); }
    void setPosition(const Eigen::Vector3d& position) override {
        setX(position.x());
        setY(position.y());
        setZ(position.z());
    }

    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject& json) override;
private:
    std::array<double, 3> m_coordinates;
};
