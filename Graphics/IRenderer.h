#ifndef IRENDERER_H
#define IRENDERER_H

#include <memory>
#include <QColor>
#include <qimage.h>
#include <Qt>

class QPainter;
class QPointF;

class GeometricObject;
class Point;
class Curve;
class ViewportManager;

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void beginRender(QPainter* painter, ViewportManager* viewportManager) = 0;
    virtual void endRender() = 0;

    virtual void renderPoint(const Point& point) = 0;
    virtual void renderCurve(const Curve& curve) = 0;

    virtual void renderObject(const GeometricObject& object) = 0;

    virtual void renderGrid(double gridSize) = 0;
    virtual void renderAxes() = 0;
    virtual void renderSelectionMarker(const QPointF& position) = 0;

    virtual void renderControlPoint(const QPointF& position, bool isSelected = false, bool isActive = false) = 0;
    virtual void renderControlPoints(const std::vector<Point>& points, int selectedIndex = -1, int activeIndex = -1) = 0;
    virtual void renderControlPolygon(const std::vector<Point>& points) = 0;

    virtual void setPointStyle(const QColor& color, float size) = 0;
    virtual void setCurveStyle(const QColor& color, float width, Qt::PenStyle style = Qt::SolidLine) = 0;
    virtual void setSelectionStyle(const QColor& color, float size) = 0;
    virtual void setGridStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) = 0;
    virtual void setAxesStyle(const QColor& color, float width) = 0;
    virtual void setControlPointStyle(const QColor& normalColor, const QColor& selectedColor, const QColor& activeColor, float size) = 0;
    virtual void setControlPolygonStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DashLine) = 0;

    virtual void setDrawGrid(bool draw) = 0;
    virtual void setDrawAxes(bool draw) = 0;
    virtual void setDrawControlPolygon(bool draw) = 0;

    virtual void renderPolyline(const std::vector<Point>& points) = 0;
    virtual void renderConvexHull(const std::vector<Point>& points) = 0;

    virtual void setPolylineStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DashLine) = 0;
    virtual void setConvexHullStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) = 0;
};

#endif
