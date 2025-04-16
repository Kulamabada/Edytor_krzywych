#pragma once

#include <memory>
#include <Eigen/Dense>
using Vector = Eigen::Vector3d;
class TransformMatrix;

class GeometricObject {
public:
    virtual ~GeometricObject() = default;

    virtual Vector getPosition() const = 0;
    virtual void setPosition(const Eigen::Vector3d& position) = 0;

    virtual void transform(const TransformMatrix& matrix) = 0;
};
