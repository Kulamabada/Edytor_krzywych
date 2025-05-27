#include "ActionHandler.h"
#include "MainWindow.h"
#include "../Core/CommandManager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QKeyEvent>
#include <QStatusBar>

ActionHandler::ActionHandler(MainWindow& mainWindow)
    : QObject(&mainWindow)
    , m_mainWindow(mainWindow)
{
    connectActions();
}

void ActionHandler::connectActions() {
    connect(m_mainWindow.m_exitAction, &QAction::triggered, this, &ActionHandler::onExitAction);
    connect(m_mainWindow.m_newAction, &QAction::triggered, this, &ActionHandler::onNewAction);
    connect(m_mainWindow.m_openAction, &QAction::triggered, this, &ActionHandler::onOpenAction);
    connect(m_mainWindow.m_saveAction, &QAction::triggered, this, &ActionHandler::onSaveAction);
    connect(m_mainWindow.m_saveAsAction, &QAction::triggered, this, &ActionHandler::onSaveAsAction);

    connect(m_mainWindow.m_undoAction, &QAction::triggered, this, &ActionHandler::onUndoAction);
    connect(m_mainWindow.m_redoAction, &QAction::triggered, this, &ActionHandler::onRedoAction);
    connect(m_mainWindow.m_deleteAction, &QAction::triggered, this, &ActionHandler::onDeleteAction);
    connect(m_mainWindow.m_selectModeAction, &QAction::triggered, this, &ActionHandler::onSelectModeAction);
    connect(m_mainWindow.m_addPointModeAction, &QAction::triggered, this, &ActionHandler::onAddPointModeAction);
    connect(m_mainWindow.m_movePointModeAction, &QAction::triggered, this, &ActionHandler::onMovePointModeAction);
    connect(m_mainWindow.m_deletePointAction, &QAction::triggered, this, &ActionHandler::onDeletePointAction);
    connect(m_mainWindow.m_finishEditingAction, &QAction::triggered, this, &ActionHandler::onFinishEditingAction);

    connect(m_mainWindow.m_zoomInAction, &QAction::triggered, this, &ActionHandler::onZoomInAction);
    connect(m_mainWindow.m_zoomOutAction, &QAction::triggered, this, &ActionHandler::onZoomOutAction);
    connect(m_mainWindow.m_resetViewAction, &QAction::triggered, this, &ActionHandler::onResetViewAction);
    connect(m_mainWindow.m_fitContentAction, &QAction::triggered, this, &ActionHandler::onFitContentAction);

    connect(m_mainWindow.m_nifsAction, &QAction::triggered, this, &ActionHandler::onNifsAction);
    connect(m_mainWindow.m_oifsAction, &QAction::triggered, this, &ActionHandler::onOifsAction);
    connect(m_mainWindow.m_lagrangeAction, &QAction::triggered, this, &ActionHandler::onLagrangeAction);
    connect(m_mainWindow.m_barycentricAction, &QAction::triggered, this, &ActionHandler::onBarycentricAction);

    connect(m_mainWindow.m_bezierAction, &QAction::triggered, this, &ActionHandler::onBezierAction);

    connect(m_mainWindow.m_aboutAction, &QAction::triggered, this, &ActionHandler::onAboutAction);
}

void ActionHandler::onNewAction() {
    m_mainWindow.getCanvas()->clearObjects();
    m_mainWindow.getCommandManager()->clearHistory();
    m_mainWindow.getCanvas()->resetView();
    m_mainWindow.updateObjectsTree();
    m_mainWindow.getStatusBar()->showMessage("Utworzono nowy projekt");
}

void ActionHandler::onOpenAction() {
    QString filePath = QFileDialog::getOpenFileName(&m_mainWindow, "Otwórz projekt",
        QString(), "Pliki projektu (*.crv);;Wszystkie pliki (*.*)");
    if (!filePath.isEmpty()) {
        m_mainWindow.getStatusBar()->showMessage("Otwieranie projektu nie jest jeszcze zaimplementowane");
    }
}

void ActionHandler::onSaveAction() {
    m_mainWindow.getStatusBar()->showMessage("Zapisywanie projektu nie jest jeszcze zaimplementowane");
}

void ActionHandler::onSaveAsAction() {
    QString filePath = QFileDialog::getSaveFileName(&m_mainWindow, "Zapisz projekt jako",
        QString(), "Pliki projektu (*.crv);;Wszystkie pliki (*.*)");
    if (!filePath.isEmpty()) {
        m_mainWindow.getStatusBar()->showMessage("Zapisywanie projektu nie jest jeszcze zaimplementowane");
    }
}

void ActionHandler::onExitAction() {
    QApplication::quit();
}

void ActionHandler::onUndoAction() {
    if (m_mainWindow.getCommandManager()->canUndo()) {
        m_mainWindow.getCommandManager()->undo();
        m_mainWindow.getCanvas()->refresh();
        m_mainWindow.updateObjectsTree();
        m_mainWindow.getStatusBar()->showMessage("Cofnięto ostatnią operację");
    }
}

void ActionHandler::onRedoAction() {
    if (m_mainWindow.getCommandManager()->canRedo()) {
        m_mainWindow.getCommandManager()->redo();
        m_mainWindow.getCanvas()->refresh();
        m_mainWindow.updateObjectsTree();
        m_mainWindow.getStatusBar()->showMessage("Ponowiono operację");
    }
}

void ActionHandler::onDeleteAction() {
    auto selectedObject = m_mainWindow.getCanvas()->selectedObject();
    if (selectedObject) {
        m_mainWindow.getCanvas()->removeObject(selectedObject);
        m_mainWindow.getCanvas()->refresh();
        m_mainWindow.updateObjectsTree();
        m_mainWindow.getStatusBar()->showMessage("Usunięto wybrany obiekt");
    }
}

void ActionHandler::onSelectModeAction() const {
    m_mainWindow.getCanvas()->setEditMode(Canvas::EditMode::SelectPoint);
    m_mainWindow.updateModeLabel();
    m_mainWindow.getStatusBar()->showMessage("Przełączono na tryb zaznaczania punktu kontrolnego");
}

void ActionHandler::onAddPointModeAction() {
    if (m_mainWindow.getCanvas()->hasSelectedCurveWithControlPoints()) {
        m_mainWindow.getCanvas()->startAddingControlPoints();
        m_mainWindow.updateModeLabel();
        m_mainWindow.getStatusBar()->showMessage("Tryb dodawania punktów kontrolnych...");
    } else {
        QMessageBox::warning(&m_mainWindow, "Edycja punktów kontrolnych",
                            "Najpierw zaznacz krzywą, aby dodać do niej punkty kontrolne.");
        m_mainWindow.m_selectModeAction->setChecked(true);
    }
}

void ActionHandler::onMovePointModeAction() {
    if (m_mainWindow.getCanvas()->hasSelectedCurveWithControlPoints()) {
        m_mainWindow.getCanvas()->startMovingControlPoints();
        m_mainWindow.updateModeLabel();
        m_mainWindow.getStatusBar()->showMessage("Tryb przesuwania punktów kontrolnych...");
    } else {
        QMessageBox::warning(&m_mainWindow, "Edycja punktów kontrolnych",
                            "Najpierw zaznacz krzywą, aby przesuwać jej punkty kontrolne.");
        m_mainWindow.m_selectModeAction->setChecked(true);
    }
}

void ActionHandler::onDeletePointAction() {
    auto selectedObj = m_mainWindow.getCanvas()->selectedObject();
    if (!selectedObj) {
        m_mainWindow.getStatusBar()->showMessage("Nie wybrano krzywej");
        return;
    }

    const bool isCurveWithControlPoints = std::dynamic_pointer_cast<InterpolatedCurve>(selectedObj) ||
        std::dynamic_pointer_cast<BezierCurve>(selectedObj);

    if (isCurveWithControlPoints) {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QApplication::sendEvent(m_mainWindow.getCanvas(), &keyEvent);

        m_mainWindow.updateObjectsTree();
        m_mainWindow.updatePropertiesPanel();
        m_mainWindow.getStatusBar()->showMessage("Usunięto punkt kontrolny");
    } else {
        m_mainWindow.getStatusBar()->showMessage("Nie wybrano krzywej lub punktu kontrolnego");
    }
}

void ActionHandler::onFinishEditingAction() {
    m_mainWindow.getCanvas()->finishEditing();
    m_mainWindow.m_selectModeAction->setChecked(true);
    m_mainWindow.updateModeLabel();
    m_mainWindow.getStatusBar()->showMessage("Zakończono edycję punktów kontrolnych");
}

void ActionHandler::onZoomInAction() {
    m_mainWindow.getCanvas()->zoomIn();
    m_mainWindow.getStatusBar()->showMessage("Przybliżono widok");
}

void ActionHandler::onZoomOutAction() {
    m_mainWindow.getCanvas()->zoomOut();
    m_mainWindow.getStatusBar()->showMessage("Oddalono widok");
}

void ActionHandler::onResetViewAction() {
    m_mainWindow.getCanvas()->resetView();
    m_mainWindow.getStatusBar()->showMessage("Zresetowano widok");
}

void ActionHandler::onFitContentAction() {
    m_mainWindow.getCanvas()->fitToContents();
    m_mainWindow.getStatusBar()->showMessage("Dopasowano widok do zawartości");
}

void ActionHandler::createInterpolatedCurve(InterpolatedCurve::InterpolationType type, const QString& typeName) {
    auto curve = std::make_shared<InterpolatedCurve>(std::static_pointer_cast<IMathProvider>(m_mainWindow.m_mathProvider), type);

    curve->setColor(Qt::blue);
    curve->setLineWidth(1.0);
    curve->setLineStyle(Qt::SolidLine);

    curve->addPoint(Point(0, 0));
    curve->addPoint(Point(50, 100));
    curve->addPoint(Point(100, 0));
    m_mainWindow.getCanvas()->addObject(curve);
    m_mainWindow.getCanvas()->selectObject(curve);
    m_mainWindow.getCanvas()->refresh();
    m_mainWindow.updateObjectsTree();
    m_mainWindow.getStatusBar()->showMessage(QString("Utworzono krzywą interpolowaną typu %1").arg(typeName));
}

void ActionHandler::onNifsAction() {
    createInterpolatedCurve(InterpolatedCurve::InterpolationType::NIFS3, "NIFS3");
}

void ActionHandler::onOifsAction() {
    createInterpolatedCurve(InterpolatedCurve::InterpolationType::OIFS3, "OIFS3");
}

void ActionHandler::onLagrangeAction() {
    createInterpolatedCurve(InterpolatedCurve::InterpolationType::Lagrange, "Lagrange");
}

void ActionHandler::onBarycentricAction() {
    createInterpolatedCurve(InterpolatedCurve::InterpolationType::Barycentric, "Barycentric");
}

void ActionHandler::onBezierAction() {
    const auto curve = std::make_shared<BezierCurve>();

    curve->addControlPoint(Point(-50, -25, 0));
    curve->addControlPoint(Point(-25, 75, 0));
    curve->addControlPoint(Point(25, 75, 0));
    curve->addControlPoint(Point(50, -25, 0));

    m_mainWindow.getCanvas()->addObject(curve);
    m_mainWindow.getCanvas()->selectObject(curve);
    m_mainWindow.getCanvas()->refresh();
    m_mainWindow.updateObjectsTree();
    m_mainWindow.getStatusBar()->showMessage("Utworzono krzywą Béziera");
}

void ActionHandler::onAboutAction() {
    QMessageBox::about(&m_mainWindow, "O programie",
        "<h3>Edytor Krzywych</h3>"
        "<p>Wersja 1.0</p>"
        "<p>Program do tworzenia i edycji krzywych matematycznych.</p>"
        "<p><b>Funkcjonalności edycji punktów kontrolnych:</b></p>"
        "<ul>"
        "<li>Dodawanie nowych punktów kontrolnych</li>"
        "<li>Przesuwanie istniejących punktów</li>"
        "<li>Usuwanie wybranych punktów</li>"
        "<li>Wizualizacja punktów kontrolnych</li>"
        "</ul>");
}
