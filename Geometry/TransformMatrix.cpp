// File: TransformMatrix.cpp
#include "TransformMatrix.h"

// StandardTransformMatrix implementation
StandardTransformMatrix::StandardTransformMatrix()
    : m_matrix(Eigen::Matrix4d::Identity()) {
}

StandardTransformMatrix::StandardTransformMatrix(const Eigen::Matrix4d& matrix)
    : m_matrix(matrix) {
}

Eigen::Matrix4d StandardTransformMatrix::getMatrix() const {
    return m_matrix;
}

void StandardTransformMatrix::setMatrix(const Eigen::Matrix4d& matrix) {
    m_matrix = matrix;
}

Eigen::Matrix4d StandardTransformMatrix::multiply(const Eigen::Matrix4d& other) const {
    return m_matrix * other;
}
