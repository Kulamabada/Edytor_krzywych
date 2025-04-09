// File: GeometricObject.h
#pragma once

#include <memory>
#include <Eigen/Dense>

namespace Geometry {
    namespace Base {

        class GeometricObject {
        public:
            virtual ~GeometricObject() = default;

            virtual Eigen::Vector3d getPosition() const = 0;
            virtual void setPosition(const Eigen::Vector3d& position) = 0;

            virtual Eigen::Vector3d getScale() const = 0;
            virtual void setScale(const Eigen::Vector3d& scale) = 0;

            virtual Eigen::Quaterniond getRotation() const = 0;
            virtual void setRotation(const Eigen::Quaterniond& rotation) = 0;
        };

    } // namespace Base
} // namespace Geometry
