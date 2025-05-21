#pragma once

#include "Curve.h"
#include <vector>
#include <memory>
#include <Eigen/Dense>
#include <QColor>

class IMathProvider;
class Point;

class InterpolatedCurve final : public Curve {
public:
enum class InterpolationType {
    NIFS3,
    OIFS3,
    Lagrange,
    Barycentric
};
    explicit InterpolatedCurve(std::shared_ptr<IMathProvider> mathProvider,
                               InterpolationType type);
    ~InterpolatedCurve() override = default;

    void addPoint(const Point& point);
    void insertPoint(size_t index, const Point& point);
    void removePoint(size_t index);
    void setPoints(const std::vector<Point>& points);
    void movePoint(size_t index, const Point& newPosition);
    const std::vector<Point>& getPoints() const;
    size_t getPointCount() const;

    void setInterpolationType(InterpolationType type);
    InterpolationType getInterpolationType() const;

    // Curve
    double getLength() const override;
    Point getPointAt(double t) const override;
    Vector getTangentAt(double t) const override;

    // GeometricObject
    void transform(const TransformMatrix& matrix) override;
    std::unique_ptr<Curve> clone() const override;
    Vector getPosition() const override;
    void setPosition(const Eigen::Vector3d& position) override;

    void setColor(const QColor& color) { m_color = color; }
    QColor getColor() const { return m_color; }

    void setLineWidth(double width) { m_lineWidth = width; }
    double getLineWidth() const { return m_lineWidth; }

    void setLineStyle(Qt::PenStyle style) { m_lineStyle = style; }
    Qt::PenStyle getLineStyle() const { return m_lineStyle; }

private:
    std::vector<Point> points;
    InterpolationType type;
    std::shared_ptr<IMathProvider> mathProvider;

    mutable std::vector<Eigen::Matrix<double, 4, 2>> coefficients;
    mutable bool needsRebuild{true};

    QColor m_color{Qt::blue};
    double m_lineWidth{1.0};
    Qt::PenStyle m_lineStyle{Qt::SolidLine};

    void buildNIFS3() const;
    void buildOIFS3() const;
    void buildLagrange() const;
    void buildBarycentric() const;

    void rebuildInterpolation() const;
};
