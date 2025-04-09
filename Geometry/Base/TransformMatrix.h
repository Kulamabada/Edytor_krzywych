// File: TransformMatrix.h
#pragma once

#include <Eigen/Dense>

namespace Geometry {
    namespace Base {

        class TransformMatrix {
        public:
            virtual ~TransformMatrix() = default;

            virtual Eigen::Matrix4d getMatrix() const = 0;
            virtual void setMatrix(const Eigen::Matrix4d& matrix) = 0;

            virtual Eigen::Matrix4d multiply(const Eigen::Matrix4d& other) const = 0;
        };

    } // namespace Base
} // namespace Geometry
