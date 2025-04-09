#pragma once

#include "../IMathProvider.h"
#include <Eigen/Dense>
#include <vector>
#include <array>

class TransformMatrix;

class EigenMathAdapter : public IMathProvider {
public:
    EigenMathAdapter();
    virtual ~EigenMathAdapter() override = default;

    // Matrix operations from IMathProvider
    TransformMatrix createRotationMatrix(double angle, const std::array<double, 3>& axis) override;
    TransformMatrix createTranslationMatrix(const std::array<double, 3>& translation) override;
    TransformMatrix createScalingMatrix(const std::array<double, 3>& factors) override;

    // Linear algebra operations
    std::vector<double> solveLinearSystem(const std::vector<std::vector<double>>& coefficients,
                                         const std::vector<double>& constants) override;

private:
    // Helper methods for Eigen conversions
    Eigen::MatrixXd toEigenMatrix(const std::vector<std::vector<double>>& matrix) const;
    std::vector<std::vector<double>> fromEigenMatrix(const Eigen::MatrixXd& eigenMatrix) const;
};
