#pragma once

#include "../Geometry/InterpolatedCurve.h"

#include "../Core/CommandManager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QStatusBar>

class MainWindow;
using Type = InterpolatedCurve::InterpolationType;

class ActionHandler : public QObject {
    Q_OBJECT

public:
    explicit ActionHandler(MainWindow& mainWindow);
    ~ActionHandler() override = default;

    public slots:
        void onNewAction();
    void onOpenAction();
    void onSaveAction();
    void onSaveAsAction();

    void onExitAction();
    void onUndoAction();
    void onRedoAction();
    void onDeleteAction();

    void onSelectModeAction() const;
    void onAddPointModeAction();
    void onMovePointModeAction();
    void onDeletePointAction();
    void onFinishEditingAction();

    void onZoomInAction();
    void onZoomOutAction();
    void onResetViewAction();
    void onFitContentAction();

    void onNifsAction();
    void onOifsAction();
    void onLagrangeAction();
    void onBarycentricAction();

    void onBezierAction();

    void onAboutAction() const;

private:
    void connectActions();
    void createInterpolatedCurve(InterpolatedCurve::InterpolationType type, const QString& typeName);
    void createAndAddCurve(std::shared_ptr<Curve> curve, const QString& message);

    MainWindow& m_mainWindow;
};
