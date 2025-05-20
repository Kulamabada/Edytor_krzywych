#include "ImageObject.h"
#include "TransformMatrix.h"

ImageObject::ImageObject(const QString& filePath) : m_image(QImage(filePath)) {
    const double width = m_image.width() / 20.0;  // Skalowanie do współrzędnych świata
    const double height = m_image.height() / 20.0;

    // Domyślnie tworzymy prostokąt o środku w (0,0)
    m_cornerPoints = {
        Point(-width/2, -height/2, 0),  // Lewy górny
        Point(width/2, -height/2, 0),   // Prawy górny
        Point(width/2, height/2, 0),    // Prawy dolny
        Point(-width/2, height/2, 0)    // Lewy dolny
    };
}

void ImageObject::setCornerPoints(const std::array<Point, 4>& points) {
    if (m_locked) return;
    m_cornerPoints = points;
}

void ImageObject::setOpacity(double opacity) {
    m_opacity = std::max(0.0, std::min(1.0, opacity));
}

void ImageObject::setLocked(bool locked) {
    m_locked = locked;
}

Vector ImageObject::getPosition() const {
    // Zwracamy środek prostokąta
    Vector sum = Vector::Zero();
    for (const auto& p : m_cornerPoints) {
        sum += Vector(p.x(), p.y(), p.z());
    }
    return sum / 4.0;
}

void ImageObject::setPosition(const Eigen::Vector3d& position) {
    if (m_locked) return;

    Vector currentPos = getPosition();
    Vector delta = position - currentPos;

    for (auto& p : m_cornerPoints) {
        p.setX(p.x() + delta.x());
        p.setY(p.y() + delta.y());
        p.setZ(p.z() + delta.z());
    }
}

void ImageObject::transform(const TransformMatrix& matrix) {
    if (m_locked) return;

    for (auto& p : m_cornerPoints) {
        p.transform(matrix);
    }
}
