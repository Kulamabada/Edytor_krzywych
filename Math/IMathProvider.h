// File: IMathProvider.h
#pragma once

#include <memory>

namespace Math {
    namespace Interfaces {

        class IMathProvider {
        public:
            virtual ~IMathProvider() = default;

            virtual double add(double a, double b) const = 0;
            virtual double subtract(double a, double b) const = 0;
            virtual double multiply(double a, double b) const = 0;
            virtual double divide(double a, double b) const = 0;
        };

    } // namespace Interfaces
} // namespace Math
