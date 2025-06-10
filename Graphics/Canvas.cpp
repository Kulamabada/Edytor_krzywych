#include "Canvas.h"

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

        const auto curve = std::dynamic_pointer_cast<Curve>(object);

        if (bool hasControlPoints = curve && curve->hasControlPoints(); m_isEditingPoints && !hasControlPoints)
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

    constexpr QRectF bounds(-10, -10, 20, 20);

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
        if ((mode == EditMode::AddPoint || mode == EditMode::MovePoint) && !hasSelectedCurveWithControlPoints()) {
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

int Canvas::hitTestControlPoint(const QPointF& pos, double tolerance) const {
    if (!m_viewportManager) return -1;

    const auto selectedObj = selectedObject();
    if (!selectedObj) return -1;

    if (const auto curve = std::dynamic_pointer_cast<Curve>(selectedObj)) {
        if (!curve->hasControlPoints()) return -1;

        const auto& points = curve->getControlPoints();
        for (size_t i = 0; i < points.size(); ++i) {
            QPointF screenPos = m_viewportManager->worldToScreen(
                QPointF(points[i].x(), points[i].y()));
            if (QLineF(pos, screenPos).length() <= tolerance)
                return static_cast<int>(i);
        }
    }
    return -1;
}

bool Canvas::addControlPoint(const QPointF& screenPos) {
    if (!m_viewportManager) return false;

    auto selectedObj = selectedObject();
    if (!selectedObj) return false;

    if (auto curve = std::dynamic_pointer_cast<Curve>(selectedObj)) {
        if (!curve->hasControlPoints()) return false;

        const QPointF worldPos = m_viewportManager->screenToWorld(screenPos);
        const Point newPoint(worldPos.x(), worldPos.y());

        int insertIndex;

        if (m_selectedControlPointIndex >= 0 &&
            m_selectedControlPointIndex < static_cast<int>(curve->getControlPointCount())) {
            insertIndex = m_selectedControlPointIndex + 1;
            curve->insertControlPoint(insertIndex, newPoint);
            } else {
                curve->addControlPoint(newPoint);
                insertIndex = static_cast<int>(curve->getControlPointCount()) - 1;
            }

        selectControlPoint(insertIndex);
        emit controlPointAdded(insertIndex);
        update();
        return true;
    }
    return false;
}

bool Canvas::moveControlPoint(int pointIndex, const QPointF& screenPos) {
    if (!m_viewportManager || pointIndex < 0) return false;

    auto selectedObj = selectedObject();
    if (!selectedObj) return false;

    if (auto curve = std::dynamic_pointer_cast<Curve>(selectedObj)) {
        if (!curve->hasControlPoints()) return false;
        if (pointIndex >= static_cast<int>(curve->getControlPointCount())) return false;

        QPointF worldPos = m_viewportManager->screenToWorld(screenPos);
        const auto& points = curve->getControlPoints();
        auto newPoint = Point(worldPos.x(), worldPos.y(), points[pointIndex].z());
        curve->moveControlPoint(pointIndex, newPoint);

        emit controlPointMoved(pointIndex);
        update();
        return true;
    }
    return false;
}

void Canvas::selectControlPoint(const int pointIndex) {
    auto selectedObj = selectedObject();
    if (!selectedObj) return;

    if (auto curve = std::dynamic_pointer_cast<Curve>(selectedObj)) {
        if (!curve->hasControlPoints()) return;

        int maxPointCount = static_cast<int>(curve->getControlPointCount());

        if (pointIndex >= 0 && pointIndex < maxPointCount) {
            m_selectedControlPointIndex = pointIndex;
            emit controlPointSelected(pointIndex);
        } else {
            m_selectedControlPointIndex = -1;
        }
    } else {
        return;
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

    for (const auto& object : m_objects)
        if (dynamic_cast<const Image*>(object.get()))
            m_renderer->renderObject(*object);

    for (const auto& object : m_objects)
        if (dynamic_cast<const Curve*>(object.get())) 
            m_renderer->renderObject(*object);

    for (const auto& object : m_objects)
        if (!dynamic_cast<const Image*>(object.get()) && !dynamic_cast<const Curve*>(object.get()))
            m_renderer->renderObject(*object);

    if (m_selectedObject)
        if (const auto* geometricObj = dynamic_cast<const GeometricObject*>(m_selectedObject.get())) {
            auto position = geometricObj->getPosition();
            QPointF selectionPos = m_viewportManager->worldToScreen(QPointF(position.x(), position.y()));
            m_renderer->renderSelectionMarker(selectionPos);
        }

    if (m_isEditingPoints && m_selectedObject)
        drawControlPoints();

    m_renderer->endRender();
}

void Canvas::drawControlPoints() {
    if (!m_renderer) return;

    const auto selectedObj = selectedObject();
    if (!selectedObj) return;

    if (const auto curve = std::dynamic_pointer_cast<Curve>(selectedObj))
        if (curve->hasControlPoints()) {
            m_renderer->renderControlPoints(*curve, m_selectedControlPointIndex,
                                          m_isDraggingPoint ? m_selectedControlPointIndex : -1);
        }
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
                    if (const auto hitObject = hitTestObject(event->pos())) {
                        selectObject(hitObject);
                        if (hasSelectedCurveWithControlPoints()) {
                            setEditMode(EditMode::SelectPoint);
                            emit editingStarted();
                        }
                        return;
                    }
                    clearSelection();
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
        auto selectedObj = selectedObject();
        if (!selectedObj) return;

        if (auto curve = std::dynamic_pointer_cast<Curve>(selectedObj)) {
            if (curve->hasControlPoints() &&
                m_selectedControlPointIndex < static_cast<int>(curve->getControlPointCount())) {

                if (curve->getControlPointCount() > 2) {
                    curve->removeControlPoint(m_selectedControlPointIndex);
                    m_selectedControlPointIndex = -1;
                    update();
                } else {
                    QMessageBox::warning(this, "Usuwanie punktu",
                        "Krzywa musi mieć co najmniej 2 punkty kontrolne.");
                }
                }
        }
    }

    QWidget::keyPressEvent(event);
}

bool Canvas::hasSelectedCurveWithControlPoints() const {
    const auto selectedObj = selectedObject();
    if (!selectedObj) return false;

    if (const auto curve = std::dynamic_pointer_cast<Curve>(selectedObj))
        return curve->hasControlPoints();

    return false;
}

std::shared_ptr<GeometricObject> Canvas::hitTestObject(const QPointF& pos, double tolerance) const {
    if (!m_viewportManager) return nullptr;

    for (auto it = m_objects.rbegin(); it != m_objects.rend(); ++it) {
        const auto& object = *it;
        if (auto curve = std::dynamic_pointer_cast<Curve>(object)) {
            constexpr int testPoints = 100;
            for (int i = 0; i <= testPoints; ++i) {
                double t = static_cast<double>(i) / testPoints;
                Point curvePoint = curve->getPointAt(t);
                QPointF screenCurvePos = m_viewportManager->worldToScreen(
                    QPointF(curvePoint.x(), curvePoint.y()));

                if (QLineF(pos, screenCurvePos).length() <= tolerance)
                    return object;
            }
        }
    }

    return nullptr;
}

void Canvas::addImage() {
    QString filePath = QFileDialog::getOpenFileName(this,
        "Wybierz obraz", QString(),
        "Obrazy (*.png *.jpg *.jpeg *.bmp *.gif);;Wszystkie pliki (*.*)");

    if (!filePath.isEmpty()) {
        addImageFromFile(filePath);
    }
}

void Canvas::addImageFromFile(const QString& filePath) {
    auto image = std::make_shared<Image>(filePath);

    if (!image->isValid()) {
        QMessageBox::warning(this, "Błąd", "Nie można wczytać obrazu z pliku: " + filePath);
        return;
    }

    if (m_viewportManager) {
        QPointF center = m_viewportManager->visibleWorldRect().center();
        image->setPosition(Vector(center.x(), center.y(), 0.0));
    }

    addObject(image);
    selectObject(image);
    update();
}

std::shared_ptr<Image> Canvas::getSelectedImage() const {
    return std::dynamic_pointer_cast<Image>(m_selectedObject);
}

bool Canvas::isImageSelected() const {
    return getSelectedImage() != nullptr;
}
