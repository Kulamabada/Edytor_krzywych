#pragma once

#include "GeometricObject.h"
#include <QPixmap>
#include <QString>
#include "TransformMatrix.h"
#include <QFileInfo>

class Image : public GeometricObject {
public:
    Image();
    explicit Image(const QString& filePath);
    explicit Image(const QPixmap& pixmap);
    ~Image() override = default;

    bool loadFromFile(const QString& filePath);
    void setPixmap(const QPixmap& pixmap);
    const QPixmap& getPixmap() const;

    void setSize(double width, double height);
    double getWidth() const;
    double getHeight() const;

    void setOpacity(double opacity);
    double getOpacity() const;

    QString getFilePath() const;
    bool isValid() const;

    Vector getPosition() const override;
    void setPosition(const Eigen::Vector3d& position) override;
    void transform(const TransformMatrix& matrix) override;

    void setLocked(bool locked) { m_locked = locked; }
    bool isLocked() const { return m_locked; }

private:
    QPixmap m_pixmap;
    QString m_filePath;
    Vector m_position{0.0, 0.0, 0.0};
    double m_width{100.0};
    double m_height{100.0};
    double m_opacity{1.0};
    bool m_locked{false};
};
