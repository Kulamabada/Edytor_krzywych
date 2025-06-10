#include "Point.h"

QJsonObject Point::toJson() const {
    QJsonObject json;
    json["x"] = x();
    json["y"] = y();
    json["z"] = z();
    return json;
}

bool Point::fromJson(const QJsonObject& json) {
    if (!json.contains("x") || !json.contains("y")) return false;

    setX(json["x"].toDouble());
    setY(json["y"].toDouble());
    setZ(json["z"].toDouble(0.0));
    return true;
}

double Point::distanceTo(const Point& other) const {
    const double dx = other.x() - x();
    const double dy = other.y() - y();
    const double dz = other.z() - z();
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Point Point::midPoint(const Point& other) const {
    return {
        (x() + other.x()) / 2.0,
        (y() + other.y()) / 2.0,
        (z() + other.z()) / 2.0
    };
}

void Point::transform(const TransformMatrix& matrix) {
    Eigen::Vector4d homogeneousPoint;
    homogeneousPoint << x(), y(), z(), 1.0;

    if (Eigen::Vector4d transformedPoint = matrix.getMatrix() * homogeneousPoint; std::abs(transformedPoint(3)) > 1e-10) {
        setX(transformedPoint(0) / transformedPoint(3));
        setY(transformedPoint(1) / transformedPoint(3));
        setZ(transformedPoint(2) / transformedPoint(3));
    }
}
