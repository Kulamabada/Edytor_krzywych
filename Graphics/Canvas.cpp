#include "Canvas.h"
#include <utility>
#include <QMessageBox>
#include "../Geometry/ImageObject.h"

Canvas::Canvas(QWidget* parent): QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_StaticContents, false);

    m_viewportManager = std::make_unique<ViewportManager>();
    if (m_viewportManager) {
        m_viewportManager->setWindowSize(width(), height());
        m_viewportManager->resetView();
    }
}

Canvas::~Canvas() = default;

void Canvas::addObject(std::shared_ptr<GeometricObject> object)
{
    if (!object) return;

    m_objects.push_back(object);
    emit objectAdded(object);
    update();
}

void Canvas::removeObject(std::shared_ptr<GeometricObject> object)
{
    if (!object) return;

    if (m_selectedObject == object)
        clearSelection();

    auto it = std::find(m_objects.begin(), m_objects.end(), object);
    if (it != m_objects.end()) {
        m_objects.erase(it);
        emit objectRemoved(object);
        update();
    }
}

void Canvas::clearObjects()
{
    m_objects.clear();
    clearSelection();
    update();
}

std::vector<std::shared_ptr<GeometricObject>> Canvas::objects() const
{
    return m_objects;
}

void Canvas::selectObject(std::shared_ptr<GeometricObject> object)
{
    if (m_selectedObject != object) {
        m_selectedObject = object;

        m_selectedControlPointIndex = -1;

        if (m_isEditingPoints && !std::dynamic_pointer_cast<InterpolatedCurve>(object))
            finishEditing();

        emit objectSelected(object);
        update();
    }
}

void Canvas::clearSelection()
{
    if (m_selectedObject) {
        m_selectedObject = nullptr;

        m_selectedControlPointIndex = -1;

        if (m_isEditingPoints)
            finishEditing();

        emit selectionCleared();
        update();
    }
}

std::shared_ptr<GeometricObject> Canvas::selectedObject() const
{
    return m_selectedObject;
}

void Canvas::zoomIn()
{
    if (m_viewportManager) {
        QPointF center(width() / 2.0, height() / 2.0);
        m_viewportManager->zoomIn(center);
        emit viewportChanged();
        update();
    }
}

void Canvas::zoomOut()
{
    if (m_viewportManager) {
        QPointF center(width() / 2.0, height() / 2.0);
        m_viewportManager->zoomOut(center);
        emit viewportChanged();
        update();
    }
}

void Canvas::resetView()
{
    if (m_viewportManager) {
        m_viewportManager->resetView();
        emit viewportChanged();
        update();
    }
}

void Canvas::fitToContents()
{
    if (m_objects.empty()) {
        resetView();
        return;
    }

    QRectF bounds(-10, -10, 20, 20);

    if (m_viewportManager) {
        m_viewportManager->fitView(bounds);
        emit viewportChanged();
        update();
    }
}

ViewportManager* Canvas::viewportManager() const
{
    return m_viewportManager.get();
}

void Canvas::setRenderer(std::shared_ptr<IRenderer> renderer)
{
    m_renderer = std::move(renderer);
    update();
}

IRenderer* Canvas::renderer() const
{
    return m_renderer.get();
}

void Canvas::refresh()
{
    if (isVisible() && width() > 0 && height() > 0 && !isMinimized()) {
        update();
    }
}

void Canvas::setEditMode(EditMode mode) {
    m_editMode = mode;

    if (mode == EditMode::None) {
        finishEditing();
    } else {
        if ((mode == EditMode::AddPoint || mode == EditMode::MovePoint) && !getSelectedCurve()) {
            m_editMode = EditMode::None;
            QMessageBox::warning(this, "Edycja punktów",
                "Najpierw wybierz krzywą do edycji punktów kontrolnych.");
            return;
        }

        m_isEditingPoints = true;
        emit editingStarted();
        update();
    }
}

Canvas::EditMode Canvas::getEditMode() const {
    return m_editMode;
}

bool Canvas::isEditingCurve() const {
    return m_isEditingPoints;
}

void Canvas::finishEditing() {
    m_editMode = EditMode::None;
    m_isEditingPoints = false;
    m_isDraggingPoint = false;
    m_selectedControlPointIndex = -1;

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
    if (!curve || !m_viewportManager) return -1;

    const auto& points = curve->getPoints();
    for (size_t i = 0; i < points.size(); ++i) {
        QPointF screenPos = m_viewportManager->worldToScreen(
            QPointF(points[i].x(), points[i].y()));

        if (QLineF(pos, screenPos).length() <= tolerance) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

bool Canvas::addControlPoint(const QPointF& screenPos) {
    const auto curve = getSelectedCurve();
    if (!curve || !m_viewportManager) return false;

    const QPointF worldPos = m_viewportManager->screenToWorld(screenPos);

    curve->addPoint(Point(worldPos.x(), worldPos.y()));

    const int newPointIndex = static_cast<int>(curve->getPointCount()) - 1;

    selectControlPoint(newPointIndex);

    emit controlPointAdded(newPointIndex);

    update();
    return true;
}

bool Canvas::moveControlPoint(int pointIndex, const QPointF& screenPos) {
    auto curve = getSelectedCurve();
    if (!curve || !m_viewportManager ||
        pointIndex < 0 || pointIndex >= static_cast<int>(curve->getPointCount())) {
        return false;
    }

    QPointF worldPos = m_viewportManager->screenToWorld(screenPos);

    const auto& points = curve->getPoints();
    auto newPoint = Point(worldPos.x(), worldPos.y(), points[pointIndex].z());
    curve->movePoint(pointIndex, newPoint);

    emit controlPointMoved(pointIndex);

    update();
    return true;
}

void Canvas::selectControlPoint(const int pointIndex) {
    const auto curve = getSelectedCurve();
    if (!curve) return;

    if (pointIndex >= 0 && pointIndex < static_cast<int>(curve->getPointCount())) {
        m_selectedControlPointIndex = pointIndex;
        emit controlPointSelected(pointIndex);
    } else {
        m_selectedControlPointIndex = -1;
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

    if (!m_renderer) return;

    m_renderer->beginRender(&painter, m_viewportManager.get());

    m_renderer->renderGrid(1.0);
    m_renderer->renderAxes();

    for (const auto& object : m_objects) {
        m_renderer->renderObject(*object);
    }

    if (m_selectedObject) {
        const auto* geometricObj = dynamic_cast<const GeometricObject*>(m_selectedObject.get());
        if (geometricObj) {
            auto position = geometricObj->getPosition();
            QPointF selectionPos = m_viewportManager->worldToScreen(QPointF(position.x(), position.y()));
            m_renderer->renderSelectionMarker(selectionPos);
        }
    }

    if (m_isEditingPoints && m_selectedObject) {
        drawControlPoints();
    }

    m_renderer->endRender();
}

void Canvas::drawControlPoints() {
    auto curve = getSelectedCurve();
    if (!curve || !m_renderer) return;

    const auto& points = curve->getPoints();

    m_renderer->renderControlPolygon(points);

    m_renderer->renderControlPoints(points, m_selectedControlPointIndex,m_isDraggingPoint ? m_selectedControlPointIndex : -1);
}

void Canvas::resizeEvent(QResizeEvent* event)
{
    if (m_viewportManager) {
        m_viewportManager->setWindowSize(event->size().width(), event->size().height());
        emit viewportChanged();
    }

    QWidget::resizeEvent(event);
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePos = event->pos();

    if (event->button() == Qt::LeftButton)
    {
        switch (m_editMode)
        {
            case EditMode::AddPoint:
                addControlPoint(event->pos());
                return;

            case EditMode::MovePoint:
                if (const int pointIndex = hitTestControlPoint(event->pos()); pointIndex >= 0) {
                    selectControlPoint(pointIndex);
                    m_isDraggingPoint = true;
                    setCursor(Qt::ClosedHandCursor);
                    return;
                }
                break;

            case EditMode::SelectPoint:
                if (const int pointIndex = hitTestControlPoint(event->pos()); pointIndex >= 0) {
                    selectControlPoint(pointIndex);
                    return;
                }
                break;

            case EditMode::None:
                    break;

            default:
                qWarning() << "Nieobsługiwany tryb edycji:" << static_cast<int>(m_editMode);
            break;
        }
    }
    else if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        setCursor(Qt::ClosedHandCursor);
    }

    QWidget::mousePressEvent(event);
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF currentPos = event->pos();
    const QPointF delta = currentPos - m_lastMousePos;
    if (m_isDraggingPoint && m_selectedControlPointIndex >= 0) {
        moveControlPoint(m_selectedControlPointIndex, currentPos);
    }
    else if (m_isPanning) {
        if (m_viewportManager) {
            m_viewportManager->pan(delta);
            emit viewportChanged();
            update();
        }
    }
    else if (m_editMode == EditMode::MovePoint) {
        if (const int pointIndex = hitTestControlPoint(currentPos); pointIndex >= 0) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }

    m_lastMousePos = currentPos;
    QWidget::mouseMoveEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isDraggingPoint) {
            m_isDraggingPoint = false;

            if (m_editMode == EditMode::MovePoint) {
                setCursor(Qt::OpenHandCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
    }
    else if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
    }

    QWidget::mouseReleaseEvent(event);
}

void Canvas::wheelEvent(QWheelEvent* event)
{
    if (m_viewportManager) {
        QPointF center = event->position();

        double factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;

        m_viewportManager->zoom(factor, center);
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
    else if (event->key() == Qt::Key_Delete && m_selectedControlPointIndex >= 0) {
        auto curve = getSelectedCurve();
        if (curve && m_selectedControlPointIndex >= 0 &&
            m_selectedControlPointIndex < static_cast<int>(curve->getPointCount())) {

            if (curve->getPointCount() > 2) {
                curve->removePoint(m_selectedControlPointIndex);
                m_selectedControlPointIndex = -1;
                update();
            } else {
                QMessageBox::warning(this, "Usuwanie punktu",
                    "Krzywa musi mieć co najmniej 2 punkty.");
            }
        }
    }

    QWidget::keyPressEvent(event);
}