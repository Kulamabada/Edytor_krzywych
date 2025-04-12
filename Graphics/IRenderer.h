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

    // Inicjalizacja i konfiguracja
    virtual void beginRender(QPainter* painter, ViewportManager* viewportManager) = 0;
    virtual void endRender() = 0;
    
    // Metody renderowania konkretnych typów obiektów
    virtual void renderPoint(const Point& point) = 0;
    virtual void renderCurve(const Curve& curve) = 0;
    
    // Ogólna metoda do renderowania obiektów geometrycznych
    virtual void renderObject(const GeometricObject& object) = 0;
    
    // Renderowanie pomocniczych elementów
    virtual void renderGrid(double gridSize) = 0;
    virtual void renderAxes() = 0;
    virtual void renderSelectionMarker(const QPointF& position) = 0;

    // Metody do ustawiania stylów rysowania
    virtual void setPointStyle(const QColor& color, float size) = 0;
    virtual void setCurveStyle(const QColor& color, float width, Qt::PenStyle style = Qt::SolidLine) = 0;
    virtual void setSelectionStyle(const QColor& color, float size) = 0;
    virtual void setGridStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) = 0;
    virtual void setAxesStyle(const QColor& color, float width) = 0;
    
    // Włączanie/wyłączanie elementów
    virtual void setDrawGrid(bool draw) = 0;
    virtual void setDrawAxes(bool draw) = 0;
};


#endif
