#include "Image.h"

Image::Image() = default;

Image::Image(const QString& filePath) {
    loadFromFile(filePath);
}

Image::Image(const QPixmap& pixmap) : m_pixmap(pixmap) {
    if (!m_pixmap.isNull()) {
        m_width = m_pixmap.width();
        m_height = m_pixmap.height();
    }
}

bool Image::loadFromFile(const QString& filePath) {
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        return false;
    }

    m_pixmap = pixmap;
    m_filePath = filePath;
    m_width = pixmap.width();
    m_height = pixmap.height();

    return true;
}

void Image::setPixmap(const QPixmap& pixmap) {
    m_pixmap = pixmap;
    if (!pixmap.isNull()) {
        m_width = pixmap.width();
        m_height = pixmap.height();
    }
}

const QPixmap& Image::getPixmap() const {
    return m_pixmap;
}

void Image::setSize(double width, double height) {
    m_width = std::max(1.0, width);
    m_height = std::max(1.0, height);
}

double Image::getWidth() const {
    return m_width;
}

double Image::getHeight() const {
    return m_height;
}

void Image::setOpacity(double opacity) {
    m_opacity = std::clamp(opacity, 0.0, 1.0);
}

double Image::getOpacity() const {
    return m_opacity;
}

QString Image::getFilePath() const {
    return m_filePath;
}

bool Image::isValid() const {
    return !m_pixmap.isNull();
}

Vector Image::getPosition() const {
    return m_position;
}

void Image::setPosition(const Eigen::Vector3d& position) {
    if (m_locked) return;
    m_position = position;
}

void Image::transform(const TransformMatrix& matrix) {
    if (m_locked) return;

    Eigen::Vector4d homogeneousPos(m_position.x(), m_position.y(), m_position.z(), 1.0);
    Eigen::Vector4d transformed = matrix.getMatrix() * homogeneousPos;

    if (std::abs(transformed(3)) > 1e-10) {
        m_position = Vector(
            transformed(0) / transformed(3),
            transformed(1) / transformed(3),
            transformed(2) / transformed(3)
        );
    }
}
