#include "EigenMathAdapter.h"
#include "TransformMatrix.h"
#include <Eigen/Dense>
#include <stdexcept>

// Pomocnicza implementacja TransformMatrix dla użycia z EigenMathAdapter
class ConcreteTransformMatrix : public Geometry::Base::TransformMatrix {
public:
    ConcreteTransformMatrix(const Eigen::Matrix4d& matrix) : m_matrix(matrix) {}

    Eigen::Matrix4d getMatrix() const override {
        return m_matrix;
    }

    void setMatrix(const Eigen::Matrix4d& matrix) override {
        m_matrix = matrix;
    }

    Eigen::Matrix4d multiply(const Eigen::Matrix4d& other) const override {
        return m_matrix * other;
    }

private:
    Eigen::Matrix4d m_matrix;
};

EigenMathAdapter::EigenMathAdapter() {
    // Inicjalizacja, jeśli potrzebna
}

// Implementacja metod z IMathProvider
double EigenMathAdapter::add(double a, double b) const {
    return a + b;
}

double EigenMathAdapter::subtract(double a, double b) const {
    return a - b;
}

double EigenMathAdapter::multiply(double a, double b) const {
    return a * b;
}

double EigenMathAdapter::divide(double a, double b) const {
    if (b == 0) {
        throw std::invalid_argument("Division by zero");
    }
    return a / b;
}

// Implementacja metod operacji na macierzach
TransformMatrix EigenMathAdapter::createRotationMatrix(double angle, const std::array<double, 3>& axis) {
    Eigen::AngleAxisd rotation(angle, Eigen::Vector3d(axis[0], axis[1], axis[2]));
    Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();
    matrix.block<3, 3>(0, 0) = rotation.toRotationMatrix();
    return ConcreteTransformMatrix(matrix);
}

TransformMatrix EigenMathAdapter::createTranslationMatrix(const std::array<double, 3>& translation) {
    Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();
    matrix(0, 3) = translation[0];
    matrix(1, 3) = translation[1];
    matrix(2, 3) = translation[2];
    return ConcreteTransformMatrix(matrix);
}

TransformMatrix EigenMathAdapter::createScalingMatrix(const std::array<double, 3>& factors) {
    Eigen::Matrix4d matrix = Eigen::Matrix4d::Identity();
    matrix(0, 0) = factors[0];
    matrix(1, 1) = factors[1];
    matrix(2, 2) = factors[2];
    return ConcreteTransformMatrix(matrix);
}

std::vector<double> EigenMathAdapter::solveLinearSystem(
    const std::vector<std::vector<double>>& coefficients,
    const std::vector<double>& constants) {

    Eigen::MatrixXd A = toEigenMatrix(coefficients);
    Eigen::VectorXd b = Eigen::VectorXd::Map(constants.data(), constants.size());

    // Użycie dekompozycji QR do rozwiązania układu równań Ax = b
    Eigen::VectorXd x = A.colPivHouseholderQr().solve(b);

    // Konwersja wyniku na std::vector
    return std::vector<double>(x.data(), x.data() + x.size());
}

Eigen::MatrixXd EigenMathAdapter::toEigenMatrix(const std::vector<std::vector<double>>& matrix) const {
    if (matrix.empty()) return Eigen::MatrixXd(0, 0);

    size_t rows = matrix.size();
    size_t cols = matrix[0].size();

    Eigen::MatrixXd result(rows, cols);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result(i, j) = matrix[i][j];
        }
    }

    return result;
}

std::vector<std::vector<double>> EigenMathAdapter::fromEigenMatrix(const Eigen::MatrixXd& eigenMatrix) const {
    std::vector<std::vector<double>> result(eigenMatrix.rows(), std::vector<double>(eigenMatrix.cols()));

    for (int i = 0; i < eigenMatrix.rows(); ++i) {
        for (int j = 0; j < eigenMatrix.cols(); ++j) {
            result[i][j] = eigenMatrix(i, j);
        }
    }

    return result;
}
