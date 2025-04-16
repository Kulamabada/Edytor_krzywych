#include "Canvas.h"

#include <utility>
#include <QMessageBox>

class Canvas::CanvasPrivate {
public:
    CanvasPrivate() : viewportManager(new ViewportManager()) {}

    std::vector<std::shared_ptr<GeometricObject>> objects;
    std::shared_ptr<GeometricObject> selectedObject;
    std::unique_ptr<ViewportManager> viewportManager;
    std::shared_ptr<IRenderer> renderer;

    bool isPanning = false;
    QPointF lastMousePos;
    QPointF selectionPoint;
    bool isSelecting = false;

    EditMode editMode = EditMode::None;
    int selectedControlPointIndex = -1;
    int activeControlPointIndex = -1;
    bool isEditingPoints = false;
    bool isDraggingPoint = false;
};

Canvas::Canvas(QWidget* parent)
    : QWidget(parent), m_dPtr(new CanvasPrivate())
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_StaticContents, false);

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

        m_dPtr->selectedControlPointIndex = -1;
        m_dPtr->activeControlPointIndex = -1;

        if (m_dPtr->isEditingPoints && !std::dynamic_pointer_cast<InterpolatedCurve>(object))
            finishEditing();

        emit objectSelected(object);
        update();
    }
}

void Canvas::clearSelection()
{
    if (m_dPtr->selectedObject) {
        m_dPtr->selectedObject = nullptr;

        m_dPtr->selectedControlPointIndex = -1;
        m_dPtr->activeControlPointIndex = -1;

        if (m_dPtr->isEditingPoints)
            finishEditing();

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

    QRectF bounds(-10, -10, 20, 20);

    if (m_dPtr->viewportManager) {
        m_dPtr->viewportManager->fitView(bounds);
        emit viewportChanged();
        update();
    }
}

ViewportManager* Canvas::viewportManager() const
{
    return m_dPtr->viewportManager.get();
}

void Canvas::setRenderer(std::shared_ptr<IRenderer> renderer)
{
    m_dPtr->renderer = std::move(renderer);
    update();
}

IRenderer* Canvas::renderer() const
{
    return m_dPtr->renderer.get();
}

void Canvas::refresh()
{
    if (isVisible() && width() > 0 && height() > 0 && !isMinimized()) {
        update();
    }
}

void Canvas::setEditMode(EditMode mode) {
    m_dPtr->editMode = mode;

    if (mode == EditMode::None) {
        finishEditing();
    } else {
        if ((mode == EditMode::AddPoint || mode == EditMode::MovePoint) && !getSelectedCurve()) {
            m_dPtr->editMode = EditMode::None;
            QMessageBox::warning(this, "Edycja punktów",
                "Najpierw wybierz krzywą do edycji punktów kontrolnych.");
            return;
        }

        m_dPtr->isEditingPoints = true;
        emit editingStarted();
        update();
    }
}

Canvas::EditMode Canvas::getEditMode() const {
    return m_dPtr->editMode;
}

bool Canvas::isEditingCurve() const {
    return m_dPtr->isEditingPoints;
}

void Canvas::finishEditing() {
    m_dPtr->editMode = EditMode::None;
    m_dPtr->isEditingPoints = false;
    m_dPtr->isDraggingPoint = false;
    m_dPtr->selectedControlPointIndex = -1;
    m_dPtr->activeControlPointIndex = -1;

    emit editingFinished();
    update();
}

std::shared_ptr<InterpolatedCurve> Canvas::getSelectedCurve() const {
    auto selectedObj = selectedObject();
    if (!selectedObj) return nullptr;

    return std::dynamic_pointer_cast<InterpolatedCurve>(selectedObj);
}

int Canvas::hitTestControlPoint(const QPointF& pos, double tolerance) {
    auto curve = getSelectedCurve();
    if (!curve || !m_dPtr->viewportManager) return -1;

    const auto& points = curve->getPoints();
    for (size_t i = 0; i < points.size(); ++i) {
        QPointF screenPos = m_dPtr->viewportManager->worldToScreen(
            QPointF(points[i].x(), points[i].y()));

        if (QLineF(pos, screenPos).length() <= tolerance) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

bool Canvas::addControlPoint(const QPointF& screenPos) {
    auto curve = getSelectedCurve();
    if (!curve || !m_dPtr->viewportManager) return false;

    QPointF worldPos = m_dPtr->viewportManager->screenToWorld(screenPos);

    curve->addPoint(Point(worldPos.x(), worldPos.y()));

    int newPointIndex = static_cast<int>(curve->getPointCount()) - 1;

    selectControlPoint(newPointIndex);

    emit controlPointAdded(newPointIndex);

    update();
    return true;
}

bool Canvas::moveControlPoint(int pointIndex, const QPointF& screenPos) {
    auto curve = getSelectedCurve();
    if (!curve || !m_dPtr->viewportManager ||
        pointIndex < 0 || pointIndex >= static_cast<int>(curve->getPointCount())) {
        return false;
    }

    QPointF worldPos = m_dPtr->viewportManager->screenToWorld(screenPos);

    const auto& points = curve->getPoints();
    auto newPoint = Point(worldPos.x(), worldPos.y(), points[pointIndex].z());
    curve->movePoint(pointIndex, newPoint);

    emit controlPointMoved(pointIndex);

    update();
    return true;
}

void Canvas::selectControlPoint(int pointIndex) {
    auto curve = getSelectedCurve();
    if (!curve) return;

    if (pointIndex >= 0 && pointIndex < static_cast<int>(curve->getPointCount())) {
        m_dPtr->selectedControlPointIndex = pointIndex;
        emit controlPointSelected(pointIndex);
    } else {
        m_dPtr->selectedControlPointIndex = -1;
    }

    update();
}

void Canvas::startAddingControlPoints() {
    setEditMode(EditMode::AddPoint);
}

void Canvas::startMovingControlPoints() {
    setEditMode(EditMode::MovePoint);
}

void Canvas::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (!isVisible() || width() <= 0 || height() <= 0) return;

    QPainter painter(this);
    if (!painter.isActive()) {
        qWarning() << "Nie można zainicjować QPainter w Canvas::paintEvent";
        return;
    }

    painter.fillRect(rect(), Qt::white);

    if (!m_dPtr->renderer) return;

    m_dPtr->renderer->beginRender(&painter, m_dPtr->viewportManager.get());

    m_dPtr->renderer->renderGrid(1.0);
    m_dPtr->renderer->renderAxes();

    for (const auto& object : m_dPtr->objects) {
        m_dPtr->renderer->renderObject(*object);
    }

    if (m_dPtr->selectedObject) {
        const auto* geometricObj = dynamic_cast<const GeometricObject*>(m_dPtr->selectedObject.get());
        if (geometricObj) {
            auto position = geometricObj->getPosition();
            QPointF selectionPos = m_dPtr->viewportManager->worldToScreen(QPointF(position.x(), position.y()));
            m_dPtr->renderer->renderSelectionMarker(selectionPos);
        }
    }

    if (m_dPtr->isEditingPoints && m_dPtr->selectedObject) {
        drawControlPoints();
    }

    m_dPtr->renderer->endRender();
}

void Canvas::drawControlPoints() {
    auto curve = getSelectedCurve();
    if (!curve || !m_dPtr->renderer) return;

    const auto& points = curve->getPoints();

    m_dPtr->renderer->renderControlPolygon(points);

    m_dPtr->renderer->renderControlPoints(points, m_dPtr->selectedControlPointIndex, m_dPtr->activeControlPointIndex);
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
        if (m_dPtr->editMode == EditMode::AddPoint) {
            addControlPoint(event->pos());
            return;
        }
        else if (m_dPtr->editMode == EditMode::MovePoint) {
            int pointIndex = hitTestControlPoint(event->pos());
            if (pointIndex >= 0) {
                selectControlPoint(pointIndex);
                m_dPtr->isDraggingPoint = true;
                m_dPtr->activeControlPointIndex = pointIndex;
                setCursor(Qt::ClosedHandCursor);
                return;
            }
        }
        else {
            m_dPtr->isSelecting = true;
            m_dPtr->selectionPoint = event->pos();
        }
    }
    else if (event->button() == Qt::MiddleButton) {
        m_dPtr->isPanning = true;
        setCursor(Qt::ClosedHandCursor);
    }

    QWidget::mousePressEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    QPointF currentPos = event->pos();
    QPointF delta = currentPos - m_dPtr->lastMousePos;

    if (m_dPtr->isDraggingPoint && m_dPtr->activeControlPointIndex >= 0) {
        moveControlPoint(m_dPtr->activeControlPointIndex, currentPos);
    }
    else if (m_dPtr->isPanning) {
        if (m_dPtr->viewportManager) {
            m_dPtr->viewportManager->pan(delta);
            emit viewportChanged();
            update();
        }
    }
    else if (m_dPtr->editMode == EditMode::MovePoint) {
        int pointIndex = hitTestControlPoint(currentPos);
        if (pointIndex >= 0) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }

    m_dPtr->lastMousePos = currentPos;
    QWidget::mouseMoveEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_dPtr->isDraggingPoint) {
            m_dPtr->isDraggingPoint = false;
            m_dPtr->activeControlPointIndex = -1;

            if (m_dPtr->editMode == EditMode::MovePoint) {
                setCursor(Qt::OpenHandCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
        else if (m_dPtr->isSelecting) {
            m_dPtr->isSelecting = false;
        }
    }
    else if (event->button() == Qt::MiddleButton) {
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

void Canvas::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        finishEditing();
    }
    else if (event->key() == Qt::Key_Delete && m_dPtr->selectedControlPointIndex >= 0) {
        auto curve = getSelectedCurve();
        if (curve && m_dPtr->selectedControlPointIndex >= 0 &&
            m_dPtr->selectedControlPointIndex < static_cast<int>(curve->getPointCount())) {

            if (curve->getPointCount() > 2) {
                curve->removePoint(m_dPtr->selectedControlPointIndex);
                m_dPtr->selectedControlPointIndex = -1;
                update();
            } else {
                QMessageBox::warning(this, "Usuwanie punktu",
                    "Krzywa musi mieć co najmniej 2 punkty.");
            }
        }
    }

    QWidget::keyPressEvent(event);
}
