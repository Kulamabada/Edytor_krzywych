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
    void renderControlPoint(const QPointF& position, const Curve& curve, bool isSelected = false, bool isActive = false) override;
    void renderControlPoints(const Curve& curve, int selectedIndex = -1, int draggingIndex = -1) override;
    void setPointStyle(const QColor& color, float size) override;
    void setCurveStyle(const QColor& color, float width, Qt::PenStyle style = Qt::SolidLine) override;
    void setSelectionStyle(const QColor& color, float size) override;
    void setGridStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) override;
    void setAxesStyle(const QColor& color, float width) override;
    void setDrawGrid(bool draw) override;
    void setDrawAxes(bool draw) override;
    void renderPolyline(const std::vector<Point>& points) override;
    void renderConvexHull(const std::vector<Point>& points) override;
    void setPolylineStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DashLine) override;
    void setConvexHullStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) override;
    void renderCurveExtras(const Curve& curve) override;
    void renderWeights(const BezierCurve& curve);

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

    QColor m_polylineColor{Qt::darkGreen};
    float m_polylineWidth{1.0f};
    Qt::PenStyle m_polylineStyle{Qt::DashLine};

    QColor m_convexHullColor{Qt::red};
    float m_convexHullWidth{1.0f};
    Qt::PenStyle m_convexHullStyle{Qt::DotLine};

    bool m_drawGrid{true};
    bool m_drawAxes{true};

    QPainter* m_painter{nullptr};
    ViewportManager* m_viewportManager{nullptr};

    QPolygonF pointsToPolygon(const std::vector<Point>& points) const;
    void renderMainCurve(const Curve& curve);
    void renderCurvePolyline(const Curve& curve);
    void renderCurveConvexHull(const Curve& curve);

    std::vector<Point> computeConvexHull(const std::vector<Point>& points) const;
    static double crossProduct(const Point& O, const Point& A, const Point& B) ;
    static double polarAngle(const Point& pivot, const Point& point) ;
    static double distanceSquared(const Point& a, const Point& b) ;
};
