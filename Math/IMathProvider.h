#pragma once

#include <memory>
#include <vector>
#include <array>

class TransformMatrix;

class IMathProvider {
public:
    virtual ~IMathProvider() = default;

    virtual std::shared_ptr<TransformMatrix> createRotationMatrix(double angle, const std::array<double, 3>& axis) = 0;
    virtual std::shared_ptr<TransformMatrix> createTranslationMatrix(const std::array<double, 3>& translation) = 0;
    virtual std::shared_ptr<TransformMatrix> createScalingMatrix(const std::array<double, 3>& factors) = 0;

    virtual std::vector<double> solveLinearSystem(
        const std::vector<std::vector<double>>& coefficients,
        const std::vector<double>& constants) = 0;
};
