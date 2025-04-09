#include "ViewportManager.h"
#include <algorithm>
#include <cmath>

namespace Graphics {

// Stałe domyślne
const double DEFAULT_MIN_ZOOM = 0.1;
const double DEFAULT_MAX_ZOOM = 100.0;
const double DEFAULT_ZOOM_FACTOR = 1.0;
const QPointF DEFAULT_PAN_OFFSET(0.0, 0.0);
const QRectF DEFAULT_VIEWPORT(-10.0, -10.0, 20.0, 20.0);

ViewportManager::ViewportManager()
    : m_viewport(DEFAULT_VIEWPORT)
    , m_windowWidth(800)
    , m_windowHeight(600)
    , m_zoomFactor(DEFAULT_ZOOM_FACTOR)
    , m_panOffset(DEFAULT_PAN_OFFSET)
    , m_minZoom(DEFAULT_MIN_ZOOM)
    , m_maxZoom(DEFAULT_MAX_ZOOM)
{
    updateTransformations();
}

ViewportManager::~ViewportManager() = default;

void ViewportManager::pan(const QPointF& delta)
{
    // Obliczenie przesunięcia w koordynatach świata
    QPointF worldDelta = screenToWorld(delta) - screenToWorld(QPointF(0, 0));

    // Aktualizacja przesunięcia
    m_panOffset -= worldDelta;

    updateTransformations();
}

void ViewportManager::zoom(double factor, const QPointF& center)
{
    // Sprawdzenie ograniczeń zoomowania
    double newZoom = m_zoomFactor * factor;
    if (newZoom < m_minZoom) {
        factor = m_minZoom / m_zoomFactor;
        newZoom = m_minZoom;
    }
    else if (newZoom > m_maxZoom) {
        factor = m_maxZoom / m_zoomFactor;
        newZoom = m_maxZoom;
    }

    // Jeśli nie ma zmiany, zakończ
    if (std::abs(factor - 1.0) < 1e-6) {
        return;
    }

    // Pobierz punkt środkowy w koordynatach świata
    QPointF worldCenter = screenToWorld(center);

    // Aktualizacja współczynnika zoomowania
    m_zoomFactor = newZoom;

    // Korekta przesunięcia, aby punkt centralny pozostał w miejscu
    QPointF newWorldCenter = screenToWorld(center);
    m_panOffset += worldCenter - newWorldCenter;

    updateTransformations();
}

void ViewportManager::resetView()
{
    // Przywrócenie domyślnych wartości
    m_zoomFactor = DEFAULT_ZOOM_FACTOR;
    m_panOffset = DEFAULT_PAN_OFFSET;
    m_viewport = DEFAULT_VIEWPORT;

    updateTransformations();
}

void ViewportManager::fitView(const QRectF& rect)
{
    if (rect.isEmpty()) {
        return;
    }

    // Obliczenie współczynnika skalowania
    double scaleX = m_windowWidth / rect.width();
    double scaleY = m_windowHeight / rect.height();

    // Użycie mniejszego współczynnika z marginesem 10%
    double newZoom = std::min(scaleX, scaleY) * 0.9;

    // Ograniczenie zoomu do dozwolonych wartości
    newZoom = std::max(m_minZoom, std::min(m_maxZoom, newZoom));

    m_zoomFactor = newZoom;
    m_panOffset = QPointF(rect.center().x(), rect.center().y());

    updateTransformations();
}

void ViewportManager::zoomIn(const QPointF& center)
{
    zoom(1.2, center); // Zbliżenie o 20%
}

void ViewportManager::zoomOut(const QPointF& center)
{
    zoom(0.8, center); // Oddalenie o 20%
}

void ViewportManager::panBy(double dx, double dy)
{
    pan(QPointF(dx, dy));
}

QPointF ViewportManager::screenToWorld(const QPointF& screenPoint) const
{
    return m_inverseViewTransform.map(screenPoint);
}

QPointF ViewportManager::worldToScreen(const QPointF& worldPoint) const
{
    return m_viewTransform.map(worldPoint);
}

QRectF ViewportManager::screenToWorld(const QRectF& screenRect) const
{
    return m_inverseViewTransform.mapRect(screenRect);
}

QRectF ViewportManager::worldToScreen(const QRectF& worldRect) const
{
    return m_viewTransform.mapRect(worldRect);
}

QTransform ViewportManager::viewTransform() const
{
    return m_viewTransform;
}

QTransform ViewportManager::inverseViewTransform() const
{
    return m_inverseViewTransform;
}

double ViewportManager::zoomFactor() const
{
    return m_zoomFactor;
}

QPointF ViewportManager::panOffset() const
{
    return m_panOffset;
}

QRectF ViewportManager::visibleWorldRect() const
{
    return screenToWorld(QRectF(0, 0, m_windowWidth, m_windowHeight));
}

void ViewportManager::setViewport(const QRectF& viewport)
{
    m_viewport = viewport;
    updateTransformations();
}

void ViewportManager::setWindowSize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
    updateTransformations();
}

void ViewportManager::setZoomFactor(double factor)
{
    m_zoomFactor = std::max(m_minZoom, std::min(m_maxZoom, factor));
    updateTransformations();
}

void ViewportManager::setPanOffset(const QPointF& offset)
{
    m_panOffset = offset;
    updateTransformations();
}

void ViewportManager::setMinZoom(double minZoom)
{
    m_minZoom = minZoom > 0 ? minZoom : DEFAULT_MIN_ZOOM;
    if (m_zoomFactor < m_minZoom) {
        m_zoomFactor = m_minZoom;
        updateTransformations();
    }
}

void ViewportManager::setMaxZoom(double maxZoom)
{
    m_maxZoom = maxZoom > m_minZoom ? maxZoom : DEFAULT_MAX_ZOOM;
    if (m_zoomFactor > m_maxZoom) {
        m_zoomFactor = m_maxZoom;
        updateTransformations();
    }
}

void ViewportManager::setZoomConstraints(double minZoom, double maxZoom)
{
    m_minZoom = minZoom > 0 ? minZoom : DEFAULT_MIN_ZOOM;
    m_maxZoom = maxZoom > m_minZoom ? maxZoom : DEFAULT_MAX_ZOOM;

    // Dostosowanie aktualnego zoomu do nowych ograniczeń
    if (m_zoomFactor < m_minZoom) {
        m_zoomFactor = m_minZoom;
    }
    else if (m_zoomFactor > m_maxZoom) {
        m_zoomFactor = m_maxZoom;
    }

    updateTransformations();
}

void ViewportManager::updateTransformations()
{
    // Zresetowanie transformacji
    m_viewTransform.reset();

    // Sekwencja transformacji:

    // 1. Przesunięcie początku układu współrzędnych do środka okna
    m_viewTransform.translate(m_windowWidth / 2.0, m_windowHeight / 2.0);

    // 2. Zastosowanie współczynnika zoomowania (odwrócenie osi Y)
    m_viewTransform.scale(m_zoomFactor, -m_zoomFactor);

    // 3. Zastosowanie przesunięcia widoku
    m_viewTransform.translate(-m_panOffset.x(), -m_panOffset.y());

    // Obliczenie transformacji odwrotnej
    m_inverseViewTransform = m_viewTransform.inverted();
}

} // namespace Graphics
