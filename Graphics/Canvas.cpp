#include "Canvas.h"
#include "ViewportManager.h"
#include "IRenderer.h"
#include "GeometricObject.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <algorithm>

class Canvas::CanvasPrivate {
public:
    CanvasPrivate() : viewportManager(new Graphics::ViewportManager()) {}

    std::vector<std::shared_ptr<GeometricObject>> objects;
    std::shared_ptr<GeometricObject> selectedObject;
    std::unique_ptr<Graphics::ViewportManager> viewportManager;
    std::shared_ptr<Graphics::IRenderer> renderer;

    // Zmienne do obsługi interakcji myszy
    bool isPanning = false;
    QPointF lastMousePos;
    QPointF selectionPoint;
    bool isSelecting = false;
};

Canvas::Canvas(QWidget* parent)
    : QWidget(parent), m_dPtr(new CanvasPrivate())
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);

    if (m_dPtr->viewportManager) {
        m_dPtr->viewportManager->setWindowSize(width(), height());
        m_dPtr->viewportManager->resetView();
    }
}

Canvas::~Canvas() = default;

void Canvas::addObject(std::shared_ptr<GeometricObject> object)
{
    if (!object) return;

    m_dPtr->objects.push_back(object);
    emit objectAdded(object);
    update();
}

void Canvas::removeObject(std::shared_ptr<GeometricObject> object)
{
    if (!object) return;

    if (m_dPtr->selectedObject == object) {
        clearSelection();
    }

    auto it = std::find(m_dPtr->objects.begin(), m_dPtr->objects.end(), object);
    if (it != m_dPtr->objects.end()) {
        m_dPtr->objects.erase(it);
        emit objectRemoved(object);
        update();
    }
}

void Canvas::clearObjects()
{
    m_dPtr->objects.clear();
    clearSelection();
    update();
}

std::vector<std::shared_ptr<GeometricObject>> Canvas::objects() const
{
    return m_dPtr->objects;
}

void Canvas::selectObject(std::shared_ptr<GeometricObject> object)
{
    if (m_dPtr->selectedObject != object) {
        m_dPtr->selectedObject = object;
        emit objectSelected(object);
        update();
    }
}

void Canvas::clearSelection()
{
    if (m_dPtr->selectedObject) {
        m_dPtr->selectedObject = nullptr;
        emit selectionCleared();
        update();
    }
}

std::shared_ptr<GeometricObject> Canvas::selectedObject() const
{
    return m_dPtr->selectedObject;
}

void Canvas::zoomIn()
{
    if (m_dPtr->viewportManager) {
        QPointF center(width() / 2.0, height() / 2.0);
        m_dPtr->viewportManager->zoomIn(center);
        emit viewportChanged();
        update();
    }
}

void Canvas::zoomOut()
{
    if (m_dPtr->viewportManager) {
        QPointF center(width() / 2.0, height() / 2.0);
        m_dPtr->viewportManager->zoomOut(center);
        emit viewportChanged();
        update();
    }
}

void Canvas::resetView()
{
    if (m_dPtr->viewportManager) {
        m_dPtr->viewportManager->resetView();
        emit viewportChanged();
        update();
    }
}

void Canvas::fitToContents()
{
    if (m_dPtr->objects.empty()) {
        resetView();
        return;
    }

    // Obliczenie granic wszystkich obiektów geometrycznych
    QRectF bounds(-10, -10, 20, 20); // Domyślne granice

    if (m_dPtr->viewportManager) {
        m_dPtr->viewportManager->fitView(bounds);
        emit viewportChanged();
        update();
    }
}

Graphics::ViewportManager* Canvas::viewportManager() const
{
    return m_dPtr->viewportManager.get();
}

void Canvas::setRenderer(std::shared_ptr<Graphics::IRenderer> renderer)
{
    m_dPtr->renderer = renderer;
    update();
}

Graphics::IRenderer* Canvas::renderer() const
{
    return m_dPtr->renderer.get();
}

void Canvas::refresh()
{
    update();
}

void Canvas::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    if (!m_dPtr->renderer) {
        return;
    }

    m_dPtr->renderer->beginRender(&painter, m_dPtr->viewportManager.get());

    // Rysowanie elementów pomocniczych
    m_dPtr->renderer->renderGrid(1.0);
    m_dPtr->renderer->renderAxes();

    // Rysowanie obiektów geometrycznych
    for (const auto& object : m_dPtr->objects) {
        m_dPtr->renderer->renderObject(*object);
    }

    // Rysowanie zaznaczenia
    if (m_dPtr->selectedObject) {
        const auto* geometricObj = dynamic_cast<const Geometry::Base::GeometricObject*>(m_dPtr->selectedObject.get());
        if (geometricObj) {
            auto position = geometricObj->getPosition();
            QPointF selectionPos = m_dPtr->viewportManager->worldToScreen(QPointF(position.x(), position.y()));
            m_dPtr->renderer->renderSelectionMarker(selectionPos);
        }
    }

    m_dPtr->renderer->endRender();
}

void Canvas::resizeEvent(QResizeEvent* event)
{
    if (m_dPtr->viewportManager) {
        m_dPtr->viewportManager->setWindowSize(event->size().width(), event->size().height());
        emit viewportChanged();
    }

    QWidget::resizeEvent(event);
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    m_dPtr->lastMousePos = event->pos();

    if (event->button() == Qt::LeftButton) {
        m_dPtr->isSelecting = true;
        m_dPtr->selectionPoint = event->pos();

        // Tutaj można dodać kod do zaznaczania obiektów pod kursorem
    }
    else if (event->button() == Qt::MiddleButton ||
            (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) {
        m_dPtr->isPanning = true;
        setCursor(Qt::ClosedHandCursor);
    }

    QWidget::mousePressEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    QPointF currentPos = event->pos();
    QPointF delta = currentPos - m_dPtr->lastMousePos;

    if (m_dPtr->isPanning) {
        if (m_dPtr->viewportManager) {
            m_dPtr->viewportManager->pan(delta);
            emit viewportChanged();
            update();
        }
    }

    m_dPtr->lastMousePos = currentPos;
    QWidget::mouseMoveEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_dPtr->isSelecting) {
            // Tutaj logika zaznaczania obiektu
            m_dPtr->isSelecting = false;
        }
    }
    else if (event->button() == Qt::MiddleButton ||
            (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) {
        m_dPtr->isPanning = false;
        setCursor(Qt::ArrowCursor);
    }

    QWidget::mouseReleaseEvent(event);
}

void Canvas::wheelEvent(QWheelEvent* event)
{
    if (m_dPtr->viewportManager) {
        QPointF center = event->position();

        double factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;

        m_dPtr->viewportManager->zoom(factor, center);
        emit viewportChanged();
        update();
    }

    event->accept();
}
