// File: TransformMatrix.cpp
#include "TransformMatrix.h"

namespace Geometry {
    namespace Base {
        // TransformMatrix to interfejs z czystymi metodami wirtualnymi,
        // więc nie wymaga bezpośredniej implementacji

        // Poniżej konkretna implementacja transformacji, która może być używana w projekcie

        class StandardTransformMatrix : public TransformMatrix {
        public:
            StandardTransformMatrix() : m_matrix(Eigen::Matrix4d::Identity()) {}

            explicit StandardTransformMatrix(const Eigen::Matrix4d& matrix) : m_matrix(matrix) {}

            // Tworzy macierz transformacji dla rotacji wokół osi
            static StandardTransformMatrix createRotation(double angle, const Eigen::Vector3d& axis) {
                Eigen::Matrix4d mat = Eigen::Matrix4d::Identity();
                Eigen::AngleAxisd rotation(angle, axis.normalized());
                mat.block<3,3>(0,0) = rotation.toRotationMatrix();
                return StandardTransformMatrix(mat);
            }

            // Tworzy macierz translacji
            static StandardTransformMatrix createTranslation(const Eigen::Vector3d& translation) {
                Eigen::Matrix4d mat = Eigen::Matrix4d::Identity();
                mat.block<3,1>(0,3) = translation;
                return StandardTransformMatrix(mat);
            }

            // Tworzy macierz skalowania
            static StandardTransformMatrix createScaling(const Eigen::Vector3d& scale) {
                Eigen::Matrix4d mat = Eigen::Matrix4d::Identity();
                mat(0,0) = scale(0);
                mat(1,1) = scale(1);
                mat(2,2) = scale(2);
                return StandardTransformMatrix(mat);
            }

            // Implementacja metod interfejsu
            Eigen::Matrix4d getMatrix() const override {
                return m_matrix;
            }

            void setMatrix(const Eigen::Matrix4d& matrix) override {
                m_matrix = matrix;
            }

            Eigen::Matrix4d multiply(const Eigen::Matrix4d& other) const override {
                return m_matrix * other;
            }

            // Dodatkowe metody pomocnicze
            StandardTransformMatrix inverse() const {
                return StandardTransformMatrix(m_matrix.inverse());
            }

            StandardTransformMatrix operator*(const StandardTransformMatrix& other) const {
                return StandardTransformMatrix(m_matrix * other.m_matrix);
            }

        private:
            Eigen::Matrix4d m_matrix;
        };

    } // namespace Base
} // namespace Geometry
