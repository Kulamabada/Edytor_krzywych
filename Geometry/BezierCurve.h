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

    // Zarządzanie punktami kontrolnymi
    void addControlPoint(const Point& point);
    void insertControlPoint(size_t index, const Point& point);
    void removeControlPoint(size_t index);
    void setControlPoints(const std::vector<Point>& points);
    void moveControlPoint(size_t index, const Point& newPosition);
    const std::vector<Point>& getControlPoints() const;
    size_t getControlPointCount() const;
    int getDegree() const;

    // Implementacja Curve
    double getLength() const override;
    Point getPointAt(double t) const override;
    Vector getTangentAt(double t) const override;
    std::unique_ptr<Curve> clone() const override;

    // Implementacja GeometricObject
    Vector getPosition() const override;
    void setPosition(const Eigen::Vector3d& position) override;
    void transform(const TransformMatrix& matrix) override;

    // Właściwości wizualne
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

    // Właściwości wizualne
    QColor m_color{Qt::blue};
    double m_lineWidth{2.0};
    Qt::PenStyle m_lineStyle{Qt::SolidLine};

    bool m_showControlPolygon{true};
    QColor m_controlPolygonColor{Qt::darkGray};
    double m_controlPolygonWidth{1.0};
    Qt::PenStyle m_controlPolygonStyle{Qt::DashLine};

    // Algorytm de Casteljau
    Point deCasteljau(double t) const;
    Vector deCasteljauDerivative(double t) const;
};
