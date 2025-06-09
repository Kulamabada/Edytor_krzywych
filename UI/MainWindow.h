#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QTreeWidget>
#include <QActionGroup>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>

#include "../Graphics/Canvas.h"
#include "../Core/CommandManager.h"

#include "../Math/EigenMathAdapter.h"
#include "../Graphics/QtRenderer.h"
#include "WeightsPanel.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequence>
#include <QPushButton>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QApplication>
#include <QTimer>

class ActionHandler;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    Canvas* getCanvas() const { return m_canvas; }
    CommandManager* getCommandManager() const { return m_commandManager.get(); }
    QStatusBar* getStatusBar() const { return statusBar(); }
    QTreeWidget* getObjectsTree() const { return m_objectsTree; }

    void updateObjectsTree();
    void updatePropertiesPanel();
    void updateModeLabel();
    WeightsPanel* getWeightsPanel() const { return m_weightsPanel; }

public slots:
    void onObjectAdded(std::shared_ptr<GeometricObject> obj);
    void onObjectRemoved(std::shared_ptr<GeometricObject> obj);
    void onObjectSelected(std::shared_ptr<GeometricObject> obj);
    void onSelectionCleared();
    void onControlPointAdded(int pointIndex);
    void onControlPointMoved(int pointIndex);
    void onControlPointSelected(int pointIndex);
    void onEditingStarted();
    void onEditingFinished();
    void onWeightChanged(int pointIndex, double weight);
    void onWeightsReset();
    void onWeightsNormalized();
    void updateWeightsPanel();

    void onShowWeightsToggled(bool show);

private:
    void createMenus();
    void createToolbars();
    void createDockWidgets();
    void setupCentralWidget();
    void connectSignals();

    Canvas* m_canvas;
    std::unique_ptr<CommandManager> m_commandManager;
    std::shared_ptr<IMathProvider> m_mathProvider;
    std::shared_ptr<IRenderer> m_renderer;

    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_curvesMenu;
    QMenu* m_imageMenu;
    QMenu* m_helpMenu;
    QMenu* m_interpolatedCurvesMenu;
    QMenu* m_controlPointsMenu;

    QToolBar* m_mainToolBar;
    QToolBar* m_editToolBar;
    QToolBar* m_imageToolBar;

    QDockWidget* m_objectsDock;
    QDockWidget* m_propertiesDock;
    QWidget* m_propertiesWidget;
    QVBoxLayout* m_propertiesLayout;
    QCheckBox* m_showWeightsCheckBox{nullptr};

    QTreeWidget* m_objectsTree;
    QLabel* m_modeLabel;

    QAction* m_newAction;
    QAction* m_openAction;
    QAction* m_saveAction;
    QAction* m_saveAsAction;
    QAction* m_exitAction;
    QAction* m_undoAction;
    QAction* m_redoAction;
    QAction* m_deleteAction;
    QAction* m_zoomInAction;
    QAction* m_zoomOutAction;
    QAction* m_resetViewAction;
    QAction* m_fitContentAction;
    QAction* m_selectModeAction;
    QAction* m_addPointModeAction;
    QAction* m_movePointModeAction;
    QAction* m_deletePointAction;
    QAction* m_finishEditingAction;
    QAction* m_nifsAction;
    QAction* m_oifsAction;
    QAction* m_lagrangeAction;
    QAction* m_barycentricAction;
    QAction* m_bezierAction;
    QAction* m_addImageAction;
    QAction* m_aboutAction;

    QActionGroup* m_editModeGroup;
    QToolButton* m_interpolatedButton;

    std::unique_ptr<ActionHandler> m_actionHandler;

    QAction* m_showWeightsPanelAction;
    QDockWidget* m_weightsDock;
    WeightsPanel* m_weightsPanel;

    void addUnifiedCurveVisibilityControls(std::shared_ptr<Curve> curve);
    void addUnifiedCurveStyleControls(std::shared_ptr<Curve> curve);

    friend class ActionHandler;
};
