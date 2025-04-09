// File: Point.cpp
#include "Point.h"
#include "TransformMatrix.h"
#include <cmath>
#include <Eigen/Dense>

Point::Point() : m_coordinates{0.0, 0.0, 0.0} {
}

Point::Point(double x, double y, double z) : m_coordinates{x, y, z} {
}

double Point::x() const {
    return m_coordinates[0];
}

double Point::y() const {
    return m_coordinates[1];
}

double Point::z() const {
    return m_coordinates[2];
}

void Point::setX(double x) {
    m_coordinates[0] = x;
}

void Point::setY(double y) {
    m_coordinates[1] = y;
}

void Point::setZ(double z) {
    m_coordinates[2] = z;
}

double Point::distanceTo(const Point& other) const {
    const double dx = other.x() - x();
    const double dy = other.y() - y();
    const double dz = other.z() - z();
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Point Point::midPoint(const Point& other) const {
    return Point(
        (x() + other.x()) / 2.0,
        (y() + other.y()) / 2.0,
        (z() + other.z()) / 2.0
    );
}

void Point::transform(const TransformMatrix& matrix) {
    // Konwertujemy punkt do formatu Eigen::Vector4d (reprezentacja homogeniczna)
    Eigen::Vector4d homogeneousPoint;
    homogeneousPoint << x(), y(), z(), 1.0;

    // Pobieramy macierz transformacji i aplikujemy transformację
    Eigen::Vector4d transformedPoint = matrix.getMatrix() * homogeneousPoint;

    // Konwertujemy z powrotem do współrzędnych kartezjańskich (z normalizacją)
    if (std::abs(transformedPoint(3)) > 1e-10) {  // Unikamy dzielenia przez zero
        setX(transformedPoint(0) / transformedPoint(3));
        setY(transformedPoint(1) / transformedPoint(3));
        setZ(transformedPoint(2) / transformedPoint(3));
    }
}

// Implementacja metod z GeometricObject
Eigen::Vector3d Point::getPosition() const {
    return Eigen::Vector3d(x(), y(), z());
}

void Point::setPosition(const Eigen::Vector3d& position) {
    setX(position.x());
    setY(position.y());
    setZ(position.z());
}

Eigen::Vector3d Point::getScale() const {
    // Punkt nie ma skali, więc zwracamy jednostkowy wektor
    return Eigen::Vector3d::Ones();
}

void Point::setScale(const Eigen::Vector3d& scale) {
    // Operacja skalowania nie ma zastosowania dla punktu
    // Implementacja pozostaje pusta
}

Eigen::Quaterniond Point::getRotation() const {
    // Punkt nie ma orientacji, zwracamy domyślny kwaternion (brak rotacji)
    return Eigen::Quaterniond::Identity();
}

void Point::setRotation(const Eigen::Quaterniond& rotation) {
    // Operacja rotacji nie ma zastosowania dla punktu
    // Implementacja pozostaje pusta
}
