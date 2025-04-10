// File: EigenMathAdapter.cpp
#include "EigenMathAdapter.h"
#include "../Geometry/TransformMatrix.h"
#include <stdexcept>

EigenMathAdapter::EigenMathAdapter() {
    // Inicjalizacja (opcjonalna)
}

// Implementacja metod operacji na macierzach
std::shared_ptr<TransformMatrix> EigenMathAdapter::createRotationMatrix(double angle, const std::array<double, 3>& axis) {
    Eigen::Vector3d eigenAxis(axis[0], axis[1], axis[2]);
    Eigen::AngleAxisd rotation(angle, eigenAxis.normalized());
    Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();
    matrix.block<3, 3>(0, 0) = rotation.toRotationMatrix();
    return std::make_shared<StandardTransformMatrix>(matrix);
}

std::shared_ptr<TransformMatrix> EigenMathAdapter::createTranslationMatrix(const std::array<double, 3>& translation) {
    Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();
    matrix(0, 3) = translation[0];
    matrix(1, 3) = translation[1];
    matrix(2, 3) = translation[2];
    return std::make_shared<StandardTransformMatrix>(matrix);
}

std::shared_ptr<TransformMatrix> EigenMathAdapter::createScalingMatrix(const std::array<double, 3>& factors) {
    Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();
    matrix(0, 0) = factors[0];
    matrix(1, 1) = factors[1];
    matrix(2, 2) = factors[2];
    return std::make_shared<StandardTransformMatrix>(matrix);
}

std::vector<double> EigenMathAdapter::solveLinearSystem(
    const std::vector<std::vector<double>>& coefficients,
    const std::vector<double>& constants) {

    // Konwersja danych wejściowych
    Eigen::MatrixXd A = toEigenMatrix(coefficients);
    Eigen::VectorXd b = Eigen::Map<const Eigen::VectorXd>(constants.data(), constants.size());

    // Rozwiązanie układu równań (Ax = b)
    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);

    // Konwersja wyniku
    return std::vector<double>(x.data(), x.data() + x.size());
}

Eigen::MatrixXd EigenMathAdapter::toEigenMatrix(const std::vector<std::vector<double>>& matrix) const {
    if (matrix.empty()) return Eigen::MatrixXd(0, 0);

    size_t rows = matrix.size();
    size_t cols = matrix[0].size();

    Eigen::MatrixXd result(rows, cols);
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols && j < matrix[i].size(); ++j) {
            result(i, j) = matrix[i][j];
        }
    }
    return result;
}
