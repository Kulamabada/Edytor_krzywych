#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <memory>
#include <vector>

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

    public slots:
        void refresh();

    signals:
        void objectAdded(std::shared_ptr<GeometricObject> object);
    void objectRemoved(std::shared_ptr<GeometricObject> object);
    void objectSelected(std::shared_ptr<GeometricObject> object);
    void selectionCleared();
    void viewportChanged();

protected:
    // Przeciążone metody QWidget
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    class CanvasPrivate;
    std::unique_ptr<CanvasPrivate> m_dPtr;
};

#endif // CANVAS_H
