#ifndef QTRENDERER_H
#define QTRENDERER_H

#include "IRenderer.h"
#include <QColor>
#include <Qt>
#include <memory>
#include "ViewportManager.h"
#include "../Geometry/GeometricObject.h"
#include "../Geometry/Point.h"
#include "../Geometry/Curve.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPointF>
#include <QTransform>
#include <vector>
#include <cmath>
#include <QDebug>

class QtRenderer : public IRenderer {
public:
    QtRenderer();
    ~QtRenderer() override;

    // Inicjalizacja i konfiguracja
    void beginRender(QPainter* painter, ViewportManager* viewportManager) override;
    void endRender() override;

    // Metody renderowania konkretnych typów obiektów
    void renderPoint(const Point& point) override;
    void renderCurve(const Curve& curve) override;

    // Ogólna metoda do renderowania obiektów geometrycznych
    void renderObject(const GeometricObject& object) override;

    // Renderowanie pomocniczych elementów
    void renderGrid(double gridSize) override;
    void renderAxes() override;
    void renderSelectionMarker(const QPointF& position) override;

    // Metody do ustawiania stylów rysowania
    void setPointStyle(const QColor& color, float size) override;
    void setCurveStyle(const QColor& color, float width, Qt::PenStyle style = Qt::SolidLine) override;
    void setSelectionStyle(const QColor& color, float size) override;
    void setGridStyle(const QColor& color, float width, Qt::PenStyle style = Qt::DotLine) override;
    void setAxesStyle(const QColor& color, float width) override;

    // Włączanie/wyłączanie elementów
    void setDrawGrid(bool draw) override;
    void setDrawAxes(bool draw) override;

private:
    class QtRendererPrivate;
    std::unique_ptr<QtRendererPrivate> m_dPtr;
};


#endif // QTRENDERER_H
