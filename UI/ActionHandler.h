#pragma once

#include <QObject>
#include <memory>
#include "../Geometry/InterpolatedCurve.h"

class MainWindow;
class QAction;

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

    void onAboutAction();

private:
    void connectActions();
    void createInterpolatedCurve(InterpolatedCurve::InterpolationType type, const QString& typeName);

    MainWindow& m_mainWindow;
};
