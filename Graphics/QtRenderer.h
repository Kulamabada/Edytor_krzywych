#pragma once

#include "IRenderer.h"
#include "ViewportManager.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPointF>
#include <QTransform>
#include <vector>
#include <QDebug>

#include "../Geometry/InterpolatedCurve.h"
#include "../Geometry/ImageObject.h"
#include "../Geometry/BezierCurve.h"

class QtRenderer : public IRenderer {
public:
    QtRenderer();
    ~QtRenderer() override;

    void beginRender(QPainter* painter, ViewportManager* viewportManager) override;
    void endRender() override;
    void renderPoint(const Point& point) override;
    void renderCurve(const Curve& curve) override;
    void renderObject(const GeometricObject& object) override;
    void renderGrid(double gridSize) override;
    void renderText(const QPointF &position, const QString &text) const;
    void renderAxes() override;
    void renderAxisTicks();
    void renderSelectionMarker(const QPointF& position) override;
    void renderControlPoint(const QPointF& position, bool isSelected = false, bool isActive = false) override;
    void renderControlPoints(const std::vector<Point>& points, int selectedIndex = -1, int draggingIndex = -1) override;
    void renderControlPolygon(const std::vector<Point>& points) override;
    void setPointStyle(const QColor& color, float size) override;
    void setCurveStyle(const QColor& color, float width, Qt::PenStyle style = Qt::SolidLine) override;
    void setSelectionStyle(const QColor& color, float size) override;
    void setGridStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) override;
    void setAxesStyle(const QColor& color, float width) override;
    void setControlPointStyle(const QColor& normalColor, const QColor& selectedColor, const QColor& activeColor, float size) override;
    void setControlPolygonStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DashLine) override;
    void setDrawGrid(bool draw) override;
    void setDrawAxes(bool draw) override;
    void setDrawControlPolygon(bool draw) override;

    void renderPolyline(const std::vector<Point>& points) override;
    void renderConvexHull(const std::vector<Point>& points) override;
    void setPolylineStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DashLine) override;
    void setConvexHullStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) override;
private:
    QColor m_pointColor{Qt::black};
    float m_pointSize{4.0f};
    QColor m_curveColor{Qt::blue};
    float m_curveWidth{1.0f};
    Qt::PenStyle m_curveStyle{Qt::SolidLine};
    QColor m_selectionColor{Qt::red};
    float m_selectionSize{6.0f};
    QColor m_gridColor{Qt::lightGray};
    float m_gridWidth{0.5f};
    Qt::PenStyle m_gridStyle{Qt::DotLine};
    QColor m_axesColor{Qt::black};
    float m_axesWidth{1.0f};
    QColor m_controlPointNormalColor{Qt::blue};
    QColor m_controlPointSelectedColor{Qt::red};
    QColor m_controlPointActiveColor{Qt::green};
    float m_controlPointSize{5.0f};
    QColor m_controlPolygonColor{Qt::darkGray};
    float m_controlPolygonWidth{1.0f};
    Qt::PenStyle m_controlPolygonStyle{Qt::DashLine};

    QColor m_polylineColor{Qt::darkGreen};
    float m_polylineWidth{1.0f};
    Qt::PenStyle m_polylineStyle{Qt::DashLine};

    QColor m_convexHullColor{Qt::red};
    float m_convexHullWidth{1.0f};
    Qt::PenStyle m_convexHullStyle{Qt::DotLine};

    bool m_drawGrid{true};
    bool m_drawAxes{true};
    bool m_drawControlPolygon{false};

    QPainter* m_painter{nullptr};
    ViewportManager* m_viewportManager{nullptr};

    std::vector<Point> computeConvexHull(const std::vector<Point>& points) const;
    static double crossProduct(const Point& O, const Point& A, const Point& B) ;
    static double polarAngle(const Point& pivot, const Point& point) ;
    static double distanceSquared(const Point& a, const Point& b) ;

    void renderBezierControlPolygon(const std::vector<Point>& points,
                              const QColor& color,
                              double width,
                              Qt::PenStyle style);
};
