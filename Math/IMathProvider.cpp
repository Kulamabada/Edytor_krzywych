#include "IMathProvider.h"
#include <stdexcept>

namespace Math {
    namespace Interfaces {

        class BasicMathProvider : public IMathProvider {
        public:
            double add(double a, double b) const override {
                return a + b;
            }

            double subtract(double a, double b) const override {
                return a - b;
            }

            double multiply(double a, double b) const override {
                return a * b;
            }

            double divide(double a, double b) const override {
                if (b == 0) {
                    throw std::invalid_argument("Division by zero");
                }
                return a / b;
            }
        };

    } // namespace Interfaces
} // namespace Math
