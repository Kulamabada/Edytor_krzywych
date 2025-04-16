#pragma once

#include "GeometricObject.h"
#include <vector>
#include <memory>

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

    // Override from GeometricObject
    void transform(const TransformMatrix& matrix) override = 0;

};
