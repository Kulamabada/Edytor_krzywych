#pragma once

#include "Point.h"
#include <QImage>

class ImageObject final : public GeometricObject {
public:
    ImageObject(const QString& filePath);
    ~ImageObject() override = default;

    // Gettery i settery
    QImage getImage() const { return m_image; }

    std::array<Point, 4> getCornerPoints() const { return m_cornerPoints; }
    void setCornerPoints(const std::array<Point, 4>& points);

    double getOpacity() const { return m_opacity; }
    void setOpacity(double opacity);

    bool isLocked() const { return m_locked; }
    void setLocked(bool locked);

    // Implementacja GeometricObject
    Vector getPosition() const override;
    void setPosition(const Eigen::Vector3d& position) override;
    void transform(const TransformMatrix& matrix) override;

private:
    QImage m_image;
    std::array<Point, 4> m_cornerPoints;
    double m_opacity{1.0};
    bool m_locked{false};
};
