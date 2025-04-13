#ifndef CANVAS_H
#define CANVAS_H

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

// Forward declarations
class GeometricObject;
class QPaintEvent;
class QMouseEvent;
class QWheelEvent;
class QResizeEvent;
class ViewportManager;
class IRenderer;

class Canvas : public QWidget {
    Q_OBJECT

public:
    // Tryby edycji
    enum class EditMode {
        None,           // Brak trybu edycji
        SelectObject,   // Wybieranie obiektów
        AddPoint,       // Dodawanie nowych punktów kontrolnych
        MovePoint       // Przesuwanie punktów kontrolnych
    };

    explicit Canvas(QWidget* parent = nullptr);
    ~Canvas() override;

    // Zarządzanie obiektami geometrycznymi
    void addObject(std::shared_ptr<GeometricObject> object);
    void removeObject(std::shared_ptr<GeometricObject> object);
    void clearObjects();
    std::vector<std::shared_ptr<GeometricObject>> objects() const;

    // Zarządzanie zaznaczeniem
    void selectObject(std::shared_ptr<GeometricObject> object);
    void clearSelection();
    std::shared_ptr<GeometricObject> selectedObject() const;

    // Zarządzanie widokiem
    void zoomIn();
    void zoomOut();
    void resetView();
    void fitToContents();

    // Dostęp do ViewportManager
    ViewportManager* viewportManager() const;

    // Ustawienie renderera
    void setRenderer(std::shared_ptr<IRenderer> renderer);
    IRenderer* renderer() const;

    // Tryb edycji
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
    // Przeciążone metody QWidget
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    // Metody pomocnicze do obsługi interakcji
    int hitTestControlPoint(const QPointF& pos, double tolerance = 10.0);
    bool addControlPoint(const QPointF& screenPos);
    bool moveControlPoint(int pointIndex, const QPointF& screenPos);
    void drawControlPoints();


    class CanvasPrivate;
    std::unique_ptr<CanvasPrivate> m_dPtr;
};

#endif // CANVAS_H
