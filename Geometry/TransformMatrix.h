// File: TransformMatrix.h
#pragma once
#include <Eigen/Dense>
#include <memory>

class TransformMatrix {
public:
    virtual ~TransformMatrix() = default;

    virtual Eigen::Matrix4d getMatrix() const = 0;
    virtual void setMatrix(const Eigen::Matrix4d& matrix) = 0;
    virtual Eigen::Matrix4d multiply(const Eigen::Matrix4d& other) const = 0;
};

// Standardowa implementacja TransformMatrix
class StandardTransformMatrix : public TransformMatrix {
public:
    StandardTransformMatrix();
    explicit StandardTransformMatrix(const Eigen::Matrix4d& matrix);

    // Implementacja metod interfejsu
    Eigen::Matrix4d getMatrix() const override;
    void setMatrix(const Eigen::Matrix4d& matrix) override;
    Eigen::Matrix4d multiply(const Eigen::Matrix4d& other) const override;

private:
    Eigen::Matrix4d m_matrix;
};
