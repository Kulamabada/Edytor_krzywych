#pragma once

#include "IMathProvider.h"
#include <Eigen/Dense>
#include <vector>
#include <array>

class TransformMatrix;

class EigenMathAdapter : public IMathProvider {
public:
    EigenMathAdapter();
    ~EigenMathAdapter() override = default;

    // Operacje macierzowe
    std::shared_ptr<TransformMatrix> createRotationMatrix(double angle, const std::array<double, 3>& axis) override;
    std::shared_ptr<TransformMatrix> createTranslationMatrix(const std::array<double, 3>& translation) override;
    std::shared_ptr<TransformMatrix> createScalingMatrix(const std::array<double, 3>& factors) override;

    // Operacje algebry liniowej
    std::vector<double> solveLinearSystem(
        const std::vector<std::vector<double>>& coefficients,
        const std::vector<double>& constants) override;

private:
    // Metoda pomocnicza do konwersji
    Eigen::MatrixXd toEigenMatrix(const std::vector<std::vector<double>>& matrix) const;
};
