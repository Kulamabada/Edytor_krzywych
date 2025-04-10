// File: IMathProvider.h
#pragma once

#include <memory>
#include <vector>
#include <array>

class TransformMatrix;

class IMathProvider {
public:
    virtual ~IMathProvider() = default;

    // Podstawowe operacje arytmetyczne (inline)
    virtual double add(double a, double b) const { return a + b; }
    virtual double subtract(double a, double b) const { return a - b; }
    virtual double multiply(double a, double b) const { return a * b; }
    virtual double divide(double a, double b) const {
        if (b == 0) throw std::invalid_argument("Division by zero");
        return a / b;
    }

    // Operacje macierzowe
    virtual std::shared_ptr<TransformMatrix> createRotationMatrix(double angle, const std::array<double, 3>& axis) = 0;
    virtual std::shared_ptr<TransformMatrix> createTranslationMatrix(const std::array<double, 3>& translation) = 0;
    virtual std::shared_ptr<TransformMatrix> createScalingMatrix(const std::array<double, 3>& factors) = 0;

    // Operacje algebry liniowej
    virtual std::vector<double> solveLinearSystem(
        const std::vector<std::vector<double>>& coefficients,
        const std::vector<double>& constants) = 0;
};
