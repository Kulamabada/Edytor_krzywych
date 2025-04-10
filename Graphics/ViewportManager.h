#ifndef VIEWPORTMANAGER_H
#define VIEWPORTMANAGER_H

#include <QPointF>
#include <QRectF>
#include <QTransform>

class ViewportManager {
public:
    ViewportManager();
    ~ViewportManager();

    // Transformacje widoku
    void pan(const QPointF& delta);
    void zoom(double factor, const QPointF& center);
    void resetView();
    void fitView(const QRectF& rect);
    
    // Pomocnicze metody transformacji
    void zoomIn(const QPointF& center);
    void zoomOut(const QPointF& center);
    void panBy(double dx, double dy);

    // Konwersje współrzędnych
    QPointF screenToWorld(const QPointF& screenPoint) const;
    QPointF worldToScreen(const QPointF& worldPoint) const;
    QRectF screenToWorld(const QRectF& screenRect) const;
    QRectF worldToScreen(const QRectF& worldRect) const;
    
    // Dostęp do właściwości widoku
    QTransform viewTransform() const;
    QTransform inverseViewTransform() const;
    double zoomFactor() const;
    QPointF panOffset() const;
    QRectF visibleWorldRect() const;
    
    // Ustawianie właściwości widoku
    void setViewport(const QRectF& viewport);
    void setWindowSize(int width, int height);
    void setZoomFactor(double factor);
    void setPanOffset(const QPointF& offset);
    
    // Ograniczenia
    void setMinZoom(double minZoom);
    void setMaxZoom(double maxZoom);
    void setZoomConstraints(double minZoom, double maxZoom);

private:
    void updateTransformations();

    QRectF m_viewport; // Obszar w koordynatach świata
    int m_windowWidth;
    int m_windowHeight;
    double m_zoomFactor;
    QPointF m_panOffset;
    QTransform m_viewTransform;
    QTransform m_inverseViewTransform;
    double m_minZoom;
    double m_maxZoom;
};

#endif // VIEWPORTMANAGER_H
