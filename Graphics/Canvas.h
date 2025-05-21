#pragma once

#include <QWidget>
#include <memory>
#include <vector>
#include "ViewportManager.h"
#include "IRenderer.h"
#include "../Geometry/GeometricObject.h"
#include "../Geometry/InterpolatedCurve.h"
#include "../Geometry/Point.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <algorithm>

#include "../Core/ApplicationCore.h"
#include "../Core/Command.h"

class Canvas : public QWidget {
    Q_OBJECT

public:
    enum class EditMode {
        None,
        SelectObject,
        AddPoint,
        MovePoint,
    };

    explicit Canvas(QWidget* parent = nullptr);
    ~Canvas() override;

    void addObject(std::shared_ptr<GeometricObject> object);
    void removeObject(std::shared_ptr<GeometricObject> object);
    void clearObjects();
    std::vector<std::shared_ptr<GeometricObject>> objects() const;

    void selectObject(std::shared_ptr<GeometricObject> object);
    void clearSelection();
    std::shared_ptr<GeometricObject> selectedObject() const;

    void zoomIn();
    void zoomOut();
    void resetView();
    void fitToContents();

    ViewportManager* viewportManager() const;

    void setRenderer(std::shared_ptr<IRenderer> renderer);
    IRenderer* renderer() const;

    void setEditMode(EditMode mode);
    EditMode getEditMode() const;
    bool isEditingCurve() const;
    void finishEditing();

    std::shared_ptr<InterpolatedCurve> getSelectedCurve() const;
    void selectControlPoint(int pointIndex);

public slots:
    void refresh();
    void startAddingControlPoints();
    void startMovingControlPoints();

signals:
    void objectAdded(std::shared_ptr<GeometricObject> object);
    void objectRemoved(std::shared_ptr<GeometricObject> object);
    void objectSelected(std::shared_ptr<GeometricObject> object);
    void selectionCleared();
    void viewportChanged();
    void controlPointAdded(int pointIndex);
    void controlPointMoved(int pointIndex);
    void controlPointSelected(int pointIndex);
    void editingStarted();
    void editingFinished();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    int hitTestControlPoint(const QPointF& pos, double tolerance = 10.0);
    bool addControlPoint(const QPointF& screenPos);
    bool moveControlPoint(int pointIndex, const QPointF& screenPos);
    void drawControlPoints();

    std::vector<std::shared_ptr<GeometricObject>> m_objects;
    std::shared_ptr<GeometricObject> m_selectedObject;
    std::unique_ptr<ViewportManager> m_viewportManager;
    std::shared_ptr<IRenderer> m_renderer;

    bool m_isPanning{false};
    QPointF m_lastMousePos;
    QPointF m_selectionPoint;
    bool m_isSelecting{false};

    EditMode m_editMode{EditMode::None};
    int m_selectedControlPointIndex{-1};
    int m_activeControlPointIndex{-1};
    bool m_isEditingPoints{false};
    bool m_isDraggingPoint{false};
    int m_selectedImageCornerIndex{-1};
};
