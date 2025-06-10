#pragma once

#include <QColor>
#include <qimage.h>
#include <Qt>
#include "../Geometry/Image.h"

class QPainter;
class QPointF;

class GeometricObject;
class Point;
class Curve;
class ViewportManager;
class BezierCurve;

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

    virtual void renderControlPoint(const QPointF& position, const Curve& curve, bool isSelected = false, bool isActive = false) = 0;
    virtual void renderControlPoints(const Curve& curve, int selectedIndex = -1, int activeIndex = -1) = 0;

    virtual void setPointStyle(const QColor& color, float size) = 0;
    virtual void setCurveStyle(const QColor& color, float width, Qt::PenStyle style = Qt::SolidLine) = 0;
    virtual void setSelectionStyle(const QColor& color, float size) = 0;
    virtual void setGridStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) = 0;
    virtual void setAxesStyle(const QColor& color, float width) = 0;

    virtual void setDrawGrid(bool draw) = 0;
    virtual void setDrawAxes(bool draw) = 0;

    virtual void renderPolyline(const std::vector<Point>& points) = 0;
    virtual void renderConvexHull(const std::vector<Point>& points) = 0;

    virtual void setPolylineStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DashLine) = 0;
    virtual void setConvexHullStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) = 0;

    virtual void renderCurveExtras(const Curve& curve) = 0;

    virtual void renderWeights(const BezierCurve& curve) = 0;

    virtual void renderImage(const Image& image) = 0;
};
