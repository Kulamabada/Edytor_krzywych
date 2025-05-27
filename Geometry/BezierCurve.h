#pragma once

#include "Curve.h"
#include "Point.h"
#include <vector>
#include <QColor>
#include <Qt>

class BezierCurve final : public Curve {
public:
    explicit BezierCurve();
    ~BezierCurve() override = default;

    int getDegree() const;

    // Curve
    double getLength() const override;
    Point getPointAt(double t) const override;
    Vector getTangentAt(double t) const override;
    std::unique_ptr<Curve> clone() const override;

    bool hasControlPoints() const override { return true; }
    void addControlPoint(const Point& point) override;
    void insertControlPoint(size_t index, const Point& point) override;
    void removeControlPoint(size_t index) override;
    void setControlPoints(const std::vector<Point>& points) override;
    void moveControlPoint(size_t index, const Point& newPosition) override;
    const std::vector<Point>& getControlPoints() const override;
    size_t getControlPointCount() const override;

    //  GeometricObject
    Vector getPosition() const override;
    void setPosition(const Eigen::Vector3d& position) override;
    void transform(const TransformMatrix& matrix) override;

    void setColor(const QColor& color) { m_color = color; }
    QColor getColor() const { return m_color; }

    void setLineWidth(double width) { m_lineWidth = width; }
    double getLineWidth() const { return m_lineWidth; }

    void setLineStyle(Qt::PenStyle style) { m_lineStyle = style; }
    Qt::PenStyle getLineStyle() const { return m_lineStyle; }

    void setShowControlPolygon(bool show) { m_showControlPolygon = show; }
    bool getShowControlPolygon() const { return m_showControlPolygon; }

    void setControlPolygonColor(const QColor& color) { m_controlPolygonColor = color; }
    QColor getControlPolygonColor() const { return m_controlPolygonColor; }

    void setControlPolygonWidth(double width) { m_controlPolygonWidth = width; }
    double getControlPolygonWidth() const { return m_controlPolygonWidth; }

    void setControlPolygonStyle(Qt::PenStyle style) { m_controlPolygonStyle = style; }
    Qt::PenStyle getControlPolygonStyle() const { return m_controlPolygonStyle; }

private:
    std::vector<Point> m_controlPoints;

    QColor m_color{Qt::blue};
    double m_lineWidth{2.0};
    Qt::PenStyle m_lineStyle{Qt::SolidLine};

    bool m_showControlPolygon{true};
    QColor m_controlPolygonColor{Qt::darkGray};
    double m_controlPolygonWidth{1.0};
    Qt::PenStyle m_controlPolygonStyle{Qt::DashLine};

    Point deCasteljau(double t) const;
    Vector deCasteljauDerivative(double t) const;
};
