#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QListWidget>
#include <QStatusBar>
#include <QLabel>
#include <cmath>

#include "Graphics/Canvas.h"
#include "Core/CommandManager.h"
#include "Geometry/InterpolatedCurve.h"
#include "Geometry/Point.h"
#include "Graphics/QtRenderer.h"
#include "Math/EigenMathAdapter.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Główne okno aplikacji
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Edytor Krzywych");
    mainWindow.resize(1024, 768);

    // Menu aplikacji
    QMenuBar *menuBar = mainWindow.menuBar();

    // Menu Plik
    QMenu *fileMenu = menuBar->addMenu("Plik");
    QAction *newAction = fileMenu->addAction("Nowy");
    newAction->setShortcut(QKeySequence::New);
    QAction *openAction = fileMenu->addAction("Otwórz...");
    openAction->setShortcut(QKeySequence::Open);
    QAction *saveAction = fileMenu->addAction("Zapisz");
    saveAction->setShortcut(QKeySequence::Save);
    QAction *saveAsAction = fileMenu->addAction("Zapisz jako...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("Wyjście");
    exitAction->setShortcut(QKeySequence::Quit);

    // Menu Edycja
    QMenu *editMenu = menuBar->addMenu("Edycja");
    QAction *undoAction = editMenu->addAction("Cofnij");
    undoAction->setShortcut(QKeySequence::Undo);
    QAction *redoAction = editMenu->addAction("Ponów");
    redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addSeparator();
    QAction *deleteAction = editMenu->addAction("Usuń");
    deleteAction->setShortcut(QKeySequence::Delete);

    // Menu Widok
    QMenu *viewMenu = menuBar->addMenu("Widok");
    QAction *zoomInAction = viewMenu->addAction("Przybliż");
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    QAction *zoomOutAction = viewMenu->addAction("Oddal");
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    QAction *resetViewAction = viewMenu->addAction("Resetuj widok");
    resetViewAction->setShortcut(QKeySequence("Ctrl+0"));
    QAction *fitContentAction = viewMenu->addAction("Dopasuj do zawartości");

    // Menu Krzywe
    QMenu *curvesMenu = menuBar->addMenu("Krzywe");
    QAction *bezierAction = curvesMenu->addAction("Krzywa Beziera");
    QAction *interpolatedAction = curvesMenu->addAction("Krzywa interpolowana");
    QAction *parametricAction = curvesMenu->addAction("Krzywa parametryczna");

    // Menu Pomoc
    QMenu *helpMenu = menuBar->addMenu("Pomoc");
    QAction *aboutAction = helpMenu->addAction("O programie");

    // Pasek narzędzi
    QToolBar *toolBar = new QToolBar("Narzędzia główne", &mainWindow);
    mainWindow.addToolBar(toolBar);

    toolBar->addAction(newAction);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addSeparator();
    toolBar->addAction(undoAction);
    toolBar->addAction(redoAction);
    toolBar->addSeparator();
    toolBar->addAction(bezierAction);
    toolBar->addAction(interpolatedAction);
    toolBar->addAction(parametricAction);
    toolBar->addSeparator();
    toolBar->addAction(zoomInAction);
    toolBar->addAction(zoomOutAction);
    toolBar->addAction(resetViewAction);

    // Centralny widget
    QWidget *centralWidget = new QWidget(&mainWindow);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Canvas do rysowania
    Canvas *canvas = new Canvas(centralWidget);
    layout->addWidget(canvas);

    auto renderer = std::make_shared<QtRenderer>();
    canvas->setRenderer(renderer);

    // Ustawienie centralnego widgetu
    mainWindow.setCentralWidget(centralWidget);

    // Doker z listą obiektów
    QDockWidget *objectsDock = new QDockWidget("Obiekty", &mainWindow);
    objectsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QListWidget *objectsList = new QListWidget(objectsDock);
    objectsDock->setWidget(objectsList);

    mainWindow.addDockWidget(Qt::RightDockWidgetArea, objectsDock);

    // Doker z właściwościami
    QDockWidget *propertiesDock = new QDockWidget("Właściwości", &mainWindow);
    propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *propertiesWidget = new QWidget(propertiesDock);
    QVBoxLayout *propertiesLayout = new QVBoxLayout(propertiesWidget);
    propertiesLayout->addWidget(new QLabel("Wybierz obiekt, aby edytować jego właściwości"));
    propertiesDock->setWidget(propertiesWidget);

    mainWindow.addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

    // Pasek statusu
    QStatusBar *statusBar = mainWindow.statusBar();
    statusBar->showMessage("Gotowy");

    // Inicjalizacja narzędzi matematycznych
    auto mathProvider = std::make_shared<EigenMathAdapter>();

    // Inicjalizacja managera poleceń
    CommandManager commandManager;

    // Połączenia akcji
    QObject::connect(exitAction, &QAction::triggered, &app, &QApplication::quit);

    QObject::connect(newAction, &QAction::triggered, [&]() {
        canvas->clearObjects();
        commandManager.clearHistory();
        canvas->resetView();
        objectsList->clear();
        statusBar->showMessage("Utworzono nowy projekt");
    });

    QObject::connect(openAction, &QAction::triggered, [&]() {
        QString filePath = QFileDialog::getOpenFileName(&mainWindow, "Otwórz projekt",
            QString(), "Pliki projektu (*.crv);;Wszystkie pliki (*.*)");
        if (!filePath.isEmpty()) {
            statusBar->showMessage("Otwieranie projektu nie jest jeszcze zaimplementowane");
        }
    });

    QObject::connect(saveAction, &QAction::triggered, [&]() {
        statusBar->showMessage("Zapisywanie projektu nie jest jeszcze zaimplementowane");
    });

    QObject::connect(saveAsAction, &QAction::triggered, [&]() {
        QString filePath = QFileDialog::getSaveFileName(&mainWindow, "Zapisz projekt jako",
            QString(), "Pliki projektu (*.crv);;Wszystkie pliki (*.*)");
        if (!filePath.isEmpty()) {
            statusBar->showMessage("Zapisywanie projektu nie jest jeszcze zaimplementowane");
        }
    });

    QObject::connect(undoAction, &QAction::triggered, [&]() {
        if (commandManager.canUndo()) {
            commandManager.undo();
            canvas->refresh();
            statusBar->showMessage("Cofnięto ostatnią operację");
        }
    });

    QObject::connect(redoAction, &QAction::triggered, [&]() {
        if (commandManager.canRedo()) {
            commandManager.redo();
            canvas->refresh();
            statusBar->showMessage("Ponowiono operację");
        }
    });

    QObject::connect(deleteAction, &QAction::triggered, [&]() {
        auto selectedObject = canvas->selectedObject();
        if (selectedObject) {
            canvas->removeObject(selectedObject);
            canvas->refresh();
            statusBar->showMessage("Usunięto wybrany obiekt");
        }
    });

    QObject::connect(zoomInAction, &QAction::triggered, [&]() {
        canvas->zoomIn();
        statusBar->showMessage("Przybliżono widok");
    });

    QObject::connect(zoomOutAction, &QAction::triggered, [&]() {
        canvas->zoomOut();
        statusBar->showMessage("Oddalono widok");
    });

    QObject::connect(resetViewAction, &QAction::triggered, [&]() {
        canvas->resetView();
        statusBar->showMessage("Zresetowano widok");
    });

    QObject::connect(fitContentAction, &QAction::triggered, [&]() {
        canvas->fitToContents();
        statusBar->showMessage("Dopasowano widok do zawartości");
    });

    QObject::connect(interpolatedAction, &QAction::triggered, [&]() {
        auto curve = std::make_shared<InterpolatedCurve>(mathProvider);
        // Dodanie przykładowych punktów
        curve->addPoint(Point(0, 0));
        curve->addPoint(Point(50, 100));
        curve->addPoint(Point(100, 0));
        canvas->addObject(curve);
        canvas->refresh();
        statusBar->showMessage("Utworzono krzywą interpolowaną");
    });

    QObject::connect(aboutAction, &QAction::triggered, [&]() {
        QMessageBox::about(&mainWindow, "O programie",
            "<h3>Edytor Krzywych</h3>"
            "<p>Wersja 1.0</p>"
            "<p>Program do tworzenia i edycji krzywych matematycznych.</p>"
            "<p>Używa biblioteki Eigen do obliczeń matematycznych.</p>");
    });

    // Aktualizacja listy obiektów
    QObject::connect(canvas, &Canvas::objectAdded, [objectsList, &statusBar](std::shared_ptr<GeometricObject> obj) {
        const QString str = "Obiekt " + QString::number(objectsList->count() + 1);
        objectsList->addItem(str);
        statusBar->showMessage("Dodano nowy obiekt");
    });

    QObject::connect(canvas, &Canvas::objectRemoved, [objectsList, canvas, &statusBar](std::shared_ptr<GeometricObject> obj) {
        objectsList->clear();
        for (size_t i = 0; i < canvas->objects().size(); ++i) {
            const QString str = "Obiekt " + QString::number(i + 1);
            objectsList->addItem(QString(str));
        }
        statusBar->showMessage("Usunięto obiekt");
    });

    QObject::connect(objectsList, &QListWidget::currentRowChanged, [canvas, objectsList, &statusBar](int row) {
        if (row >= 0 && row < static_cast<int>(canvas->objects().size())) {
            canvas->selectObject(canvas->objects()[row]);
            canvas->refresh();
            const QString message = "Wybrano obiekt " + QString::number(row + 1);
            statusBar->showMessage(message);
        }
    });

    QObject::connect(canvas, &Canvas::objectSelected, [canvas, objectsList](std::shared_ptr<GeometricObject> obj) {
        auto objects = canvas->objects();
        auto it = std::find(objects.begin(), objects.end(), obj);
        if (it != objects.end()) {
            int row = static_cast<int>(std::distance(objects.begin(), it));
            objectsList->setCurrentRow(row);
        }
    });

    // Wyświetlenie okna
    mainWindow.show();
    statusBar->showMessage("Aplikacja gotowa do użycia");

    // Uruchomienie pętli zdarzeń aplikacji
    return app.exec();
}
