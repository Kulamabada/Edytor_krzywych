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
#include <QTreeWidget>
#include <QKeySequence>
#include <QActionGroup>
#include <QIcon>
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
    QAction *newAction = fileMenu->addAction(QIcon::fromTheme("document-new"), "Nowy");
    newAction->setShortcut(QKeySequence::New);
    QAction *openAction = fileMenu->addAction(QIcon::fromTheme("document-open"), "Otwórz...");
    openAction->setShortcut(QKeySequence::Open);
    QAction *saveAction = fileMenu->addAction(QIcon::fromTheme("document-save"), "Zapisz");
    saveAction->setShortcut(QKeySequence::Save);
    QAction *saveAsAction = fileMenu->addAction(QIcon::fromTheme("document-save-as"), "Zapisz jako...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(QIcon::fromTheme("application-exit"), "Wyjście");
    exitAction->setShortcut(QKeySequence::Quit);

    // Menu Edycja
    QMenu *editMenu = menuBar->addMenu("Edycja");
    QAction *undoAction = editMenu->addAction(QIcon::fromTheme("edit-undo"), "Cofnij");
    undoAction->setShortcut(QKeySequence::Undo);
    QAction *redoAction = editMenu->addAction(QIcon::fromTheme("edit-redo"), "Ponów");
    redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addSeparator();
    QAction *deleteAction = editMenu->addAction(QIcon::fromTheme("edit-delete"), "Usuń");
    deleteAction->setShortcut(QKeySequence::Delete);

    // Nowe podmenu do edycji punktów kontrolnych
    editMenu->addSeparator();
    QMenu *controlPointsMenu = editMenu->addMenu("Edycja punktów kontrolnych");

    QAction *selectModeAction = controlPointsMenu->addAction(QIcon::fromTheme("edit-select"), "Zaznacz punkt");
    selectModeAction->setCheckable(true);
    selectModeAction->setShortcut(QKeySequence("Ctrl+1"));

    QAction *addPointModeAction = controlPointsMenu->addAction(QIcon::fromTheme("list-add"), "Dodaj punkt");
    addPointModeAction->setCheckable(true);
    addPointModeAction->setShortcut(QKeySequence("Ctrl+2"));

    QAction *movePointModeAction = controlPointsMenu->addAction(QIcon::fromTheme("transform-move"), "Przesuń punkt");
    movePointModeAction->setCheckable(true);
    movePointModeAction->setShortcut(QKeySequence("Ctrl+3"));

    QAction *deletePointAction = controlPointsMenu->addAction(QIcon::fromTheme("edit-delete"), "Usuń punkt");
    deletePointAction->setShortcut(QKeySequence("Ctrl+D"));

    QAction *finishEditingAction = controlPointsMenu->addAction(QIcon::fromTheme("dialog-cancel"), "Zakończ edycję");
    finishEditingAction->setShortcut(QKeySequence("Esc"));

    // Grupa akcji do trybów edycji punktów (tylko jeden może być aktywny)
    QActionGroup *editModeGroup = new QActionGroup(&mainWindow);
    editModeGroup->addAction(selectModeAction);
    editModeGroup->addAction(addPointModeAction);
    editModeGroup->addAction(movePointModeAction);
    selectModeAction->setChecked(true); // Domyślnie tryb zaznaczania

    // Menu Widok
    QMenu *viewMenu = menuBar->addMenu("Widok");
    QAction *zoomInAction = viewMenu->addAction(QIcon::fromTheme("zoom-in"), "Przybliż");
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    QAction *zoomOutAction = viewMenu->addAction(QIcon::fromTheme("zoom-out"), "Oddal");
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    QAction *resetViewAction = viewMenu->addAction(QIcon::fromTheme("zoom-original"), "Resetuj widok");
    resetViewAction->setShortcut(QKeySequence("Ctrl+0"));
    QAction *fitContentAction = viewMenu->addAction(QIcon::fromTheme("zoom-fit-best"), "Dopasuj do zawartości");

    // Menu Krzywe
    QMenu *curvesMenu = menuBar->addMenu("Krzywe");
    QAction *bezierAction = curvesMenu->addAction("Krzywa Beziera");
    QAction *interpolatedAction = curvesMenu->addAction("Krzywa interpolowana");
    QAction *parametricAction = curvesMenu->addAction("Krzywa parametryczna");

    // Menu Pomoc
    QMenu *helpMenu = menuBar->addMenu("Pomoc");
    QAction *aboutAction = helpMenu->addAction(QIcon::fromTheme("help-about"), "O programie");

    // Pasek narzędzi głównych
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

    // Pasek narzędzi edycji punktów kontrolnych
    QToolBar *editToolBar = new QToolBar("Narzędzia edycji punktów", &mainWindow);
    mainWindow.addToolBar(Qt::TopToolBarArea, editToolBar);

    editToolBar->addAction(selectModeAction);
    editToolBar->addAction(addPointModeAction);
    editToolBar->addAction(movePointModeAction);
    editToolBar->addSeparator();
    editToolBar->addAction(deletePointAction);
    editToolBar->addAction(finishEditingAction);

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

    // Doker z listą obiektów i punktów kontrolnych
    QDockWidget *objectsDock = new QDockWidget("Obiekty i punkty kontrolne", &mainWindow);
    objectsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // Zamiast prostej listy, używamy drzewa dla hierarchii obiektów i punktów
    QTreeWidget *objectsTree = new QTreeWidget(objectsDock);
    objectsTree->setHeaderLabels(QStringList() << "Nazwa" << "Współrzędne");
    objectsTree->setColumnCount(2);
    objectsTree->setRootIsDecorated(true);
    objectsTree->setExpandsOnDoubleClick(true);
    objectsDock->setWidget(objectsTree);

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
    QLabel *modeLabel = new QLabel("Tryb: Normalny");
    statusBar->addPermanentWidget(modeLabel);
    statusBar->showMessage("Gotowy");

    // Inicjalizacja narzędzi matematycznych
    auto mathProvider = std::make_shared<EigenMathAdapter>();

    // Inicjalizacja managera poleceń
    CommandManager commandManager;

    // Funkcja do aktualizacji drzewa obiektów
    auto updateObjectsTree = [objectsTree, canvas]() {
        objectsTree->clear();

        // Dodaj wszystkie obiekty geometryczne do drzewa
        int index = 0;
        for (const auto& obj : canvas->objects()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(objectsTree);
            item->setText(0, QString("Obiekt %1").arg(std::to_string(index + 1)));

            // Sprawdź typ obiektu
            auto curve = std::dynamic_pointer_cast<InterpolatedCurve>(obj);
            if (curve) {
                item->setText(1, "Krzywa");

                // Dodaj punkty kontrolne jako dzieci
                const auto& points = curve->getPoints();
                for (size_t i = 0; i < points.size(); ++i) {
                    QTreeWidgetItem* pointItem = new QTreeWidgetItem(item);
                    const Point& p = points[i];
                    pointItem->setText(0, QString("Punkt %1").arg(std::to_string(i)));
                    pointItem->setText(1, QString("(%1, %2)")
                        .arg(QString::number(p.x(), 'f', 2))
                        .arg(QString::number(p.y(), 'f', 2)));

                    pointItem->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<int>(i)));
                    pointItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(index));
                }
            } else {
                item->setText(1, "Inny");
            }

            item->setData(0, Qt::UserRole, QVariant::fromValue(index));
            index++;
        }

        objectsTree->expandAll();
    };

    // Funkcja do aktualizacji etykiety trybu w pasku statusu
    auto updateModeLabel = [modeLabel, canvas]() {
        switch (canvas->getEditMode()) {
            case Canvas::EditMode::None:
                modeLabel->setText("Tryb: Normalny");
                break;
            case Canvas::EditMode::SelectObject:
                modeLabel->setText("Tryb: Zaznaczanie obiektu");
                break;
            case Canvas::EditMode::AddPoint:
                modeLabel->setText("Tryb: Dodawanie punktu");
                break;
            case Canvas::EditMode::MovePoint:
                modeLabel->setText("Tryb: Przesuwanie punktu");
                break;
        }
    };

    // Połączenia akcji podstawowych
    QObject::connect(exitAction, &QAction::triggered, &app, &QApplication::quit);

    QObject::connect(newAction, &QAction::triggered, [&]() {
        canvas->clearObjects();
        commandManager.clearHistory();
        canvas->resetView();
        updateObjectsTree();
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
            updateObjectsTree();
            statusBar->showMessage("Cofnięto ostatnią operację");
        }
    });

    QObject::connect(redoAction, &QAction::triggered, [&]() {
        if (commandManager.canRedo()) {
            commandManager.redo();
            canvas->refresh();
            updateObjectsTree();
            statusBar->showMessage("Ponowiono operację");
        }
    });

    QObject::connect(deleteAction, &QAction::triggered, [&]() {
        auto selectedObject = canvas->selectedObject();
        if (selectedObject) {
            canvas->removeObject(selectedObject);
            canvas->refresh();
            updateObjectsTree();
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
        updateObjectsTree();
        statusBar->showMessage("Utworzono krzywą interpolowaną");
    });

    QObject::connect(aboutAction, &QAction::triggered, [&]() {
        QMessageBox::about(&mainWindow, "O programie",
            "<h3>Edytor Krzywych</h3>"
            "<p>Wersja 1.0</p>"
            "<p>Program do tworzenia i edycji krzywych matematycznych.</p>"
            "<p>Używa biblioteki Eigen do obliczeń matematycznych.</p>"
            "<p><b>Funkcjonalności edycji punktów kontrolnych:</b></p>"
            "<ul>"
            "<li>Dodawanie nowych punktów kontrolnych</li>"
            "<li>Przesuwanie istniejących punktów</li>"
            "<li>Usuwanie wybranych punktów</li>"
            "<li>Wizualizacja punktów kontrolnych</li>"
            "</ul>");
    });

    // Połączenia dla akcji edycji punktów kontrolnych
    QObject::connect(selectModeAction, &QAction::triggered, [&]() {
        canvas->setEditMode(Canvas::EditMode::SelectObject);
        updateModeLabel();
        statusBar->showMessage("Przełączono na tryb zaznaczania obiektu");
    });

    QObject::connect(addPointModeAction, &QAction::triggered, [&]() {
        if (canvas->getSelectedCurve()) {
            canvas->startAddingControlPoints();
            updateModeLabel();
            statusBar->showMessage("Tryb dodawania punktów kontrolnych. Kliknij w obszarze rysowania, aby dodać punkt.");
        } else {
            QMessageBox::warning(&mainWindow, "Edycja punktów kontrolnych",
                                "Najpierw zaznacz krzywą, aby dodać do niej punkty kontrolne.");
            selectModeAction->setChecked(true);
        }
    });

    QObject::connect(movePointModeAction, &QAction::triggered, [&]() {
        if (canvas->getSelectedCurve()) {
            canvas->startMovingControlPoints();
            updateModeLabel();
            statusBar->showMessage("Tryb przesuwania punktów kontrolnych. Kliknij i przeciągnij punkt kontrolny.");
        } else {
            QMessageBox::warning(&mainWindow, "Edycja punktów kontrolnych",
                                "Najpierw zaznacz krzywą, aby przesuwać jej punkty kontrolne.");
            selectModeAction->setChecked(true);
        }
    });

    QObject::connect(deletePointAction, &QAction::triggered, [&]() {
        // Usunięcie aktualnie zaznaczonego punktu kontrolnego
        auto curve = canvas->getSelectedCurve();
        if (curve) {
            // Wysyłamy zdarzenie Delete do canvas, aby usunąć zaznaczony punkt
            QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
            QApplication::sendEvent(canvas, &keyEvent);

            updateObjectsTree();
            statusBar->showMessage("Usunięto punkt kontrolny");
        } else {
            statusBar->showMessage("Nie wybrano krzywej lub punktu kontrolnego");
        }
    });

    QObject::connect(finishEditingAction, &QAction::triggered, [&]() {
        canvas->finishEditing();
        selectModeAction->setChecked(true);
        updateModeLabel();
        statusBar->showMessage("Zakończono edycję punktów kontrolnych");
    });

    // Obsługa sygnałów z Canvas
    QObject::connect(canvas, &Canvas::objectAdded, [&](std::shared_ptr<GeometricObject> obj) {
        updateObjectsTree();
        statusBar->showMessage("Dodano nowy obiekt");
    });

    QObject::connect(canvas, &Canvas::objectRemoved, [&](std::shared_ptr<GeometricObject> obj) {
        updateObjectsTree();
        statusBar->showMessage("Usunięto obiekt");
    });

    QObject::connect(canvas, &Canvas::objectSelected, [&](std::shared_ptr<GeometricObject> obj) {
        updateObjectsTree();

        // Znajdź element w drzewie odpowiadający wybranemu obiektowi
        auto objects = canvas->objects();
        auto it = std::find(objects.begin(), objects.end(), obj);
        if (it != objects.end()) {
            int index = static_cast<int>(std::distance(objects.begin(), it));

            // Znajdź element w drzewie i zaznacz go
            for (int i = 0; i < objectsTree->topLevelItemCount(); ++i) {
                QTreeWidgetItem* item = objectsTree->topLevelItem(i);
                if (item->data(0, Qt::UserRole).toInt() == index) {
                    objectsTree->setCurrentItem(item);
                    break;
                }
            }
        }

        statusBar->showMessage("Wybrano obiekt");
    });

    QObject::connect(canvas, &Canvas::selectionCleared, [&]() {
        objectsTree->clearSelection();
        statusBar->showMessage("Anulowano zaznaczenie");
    });

    QObject::connect(canvas, &Canvas::controlPointAdded, [&](int pointIndex) {
        updateObjectsTree();
        statusBar->showMessage(QString("Dodano punkt kontrolny %1").arg(std::to_string(pointIndex)));
    });

    QObject::connect(canvas, &Canvas::controlPointMoved, [&](int pointIndex) {
        updateObjectsTree();
        statusBar->showMessage(QString("Przesunięto punkt kontrolny %1").arg(std::to_string(pointIndex)));
    });

    QObject::connect(canvas, &Canvas::controlPointSelected, [&](int pointIndex) {
        updateObjectsTree();

        // Znajdź element w drzewie odpowiadający wybranemu punktowi kontrolnemu
        auto selectedObj = canvas->selectedObject();
        auto objects = canvas->objects();
        auto it = std::find(objects.begin(), objects.end(), selectedObj);

        if (it != objects.end()) {
            int objectIndex = static_cast<int>(std::distance(objects.begin(), it));

            // Znajdź element obiektu w drzewie
            for (int i = 0; i < objectsTree->topLevelItemCount(); ++i) {
                QTreeWidgetItem* objectItem = objectsTree->topLevelItem(i);
                if (objectItem->data(0, Qt::UserRole).toInt() == objectIndex) {
                    // Znajdź element punktu kontrolnego
                    for (int j = 0; j < objectItem->childCount(); ++j) {
                        QTreeWidgetItem* pointItem = objectItem->child(j);
                        if (pointItem->data(0, Qt::UserRole).toInt() == pointIndex) {
                            objectsTree->setCurrentItem(pointItem);
                            break;
                        }
                    }
                    break;
                }
            }
        }

        statusBar->showMessage(QString("Wybrano punkt kontrolny %1").arg(std::to_string(pointIndex)));
    });

    QObject::connect(canvas, &Canvas::editingStarted, [&]() {
        updateModeLabel();
        statusBar->showMessage("Rozpoczęto edycję punktów kontrolnych");
    });

    QObject::connect(canvas, &Canvas::editingFinished, [&]() {
        selectModeAction->setChecked(true);
        updateModeLabel();
        statusBar->showMessage("Zakończono edycję punktów kontrolnych");
    });

    // Obsługa zaznaczania w drzewie obiektów
    QObject::connect(objectsTree, &QTreeWidget::itemClicked, [&](QTreeWidgetItem* item, int column) {
        // Sprawdź, czy kliknięto element najwyższego poziomu (obiekt)
        if (item->parent() == nullptr) {
            int index = item->data(0, Qt::UserRole).toInt();
            if (index >= 0 && index < static_cast<int>(canvas->objects().size())) {
                canvas->selectObject(canvas->objects()[index]);
                canvas->finishEditing();
                selectModeAction->setChecked(true);
                updateModeLabel();
            }
        }
        // Sprawdź, czy kliknięto element podrzędny (punkt kontrolny)
        else {
            int pointIndex = item->data(0, Qt::UserRole).toInt();
            int objectIndex = item->data(0, Qt::UserRole + 1).toInt();

            if (objectIndex >= 0 && objectIndex < static_cast<int>(canvas->objects().size())) {
                // Zaznacz najpierw obiekt (krzywą)
                canvas->selectObject(canvas->objects()[objectIndex]);

                // Przełącz na tryb edycji punktów i zaznacz punkt
                auto curve = canvas->getSelectedCurve();
                if (curve && pointIndex >= 0 && pointIndex < static_cast<int>(curve->getPointCount())) {
                    canvas->startMovingControlPoints();
                    movePointModeAction->setChecked(true);
                    canvas->selectControlPoint(pointIndex);
                    updateModeLabel();
                }
            }
        }
    });

    // Obsługa podwójnego kliknięcia w drzewie obiektów dla szybkiej edycji
    QObject::connect(objectsTree, &QTreeWidget::itemDoubleClicked, [&](QTreeWidgetItem* item, int column) {
        // Jeśli kliknięto punkt kontrolny
        if (item->parent() != nullptr) {
            int pointIndex = item->data(0, Qt::UserRole).toInt();
            int objectIndex = item->data(0, Qt::UserRole + 1).toInt();

            if (objectIndex >= 0 && objectIndex < static_cast<int>(canvas->objects().size())) {
                // Zaznacz obiekt
                canvas->selectObject(canvas->objects()[objectIndex]);

                // Przełącz na tryb przesuwania punktów i zaznacz punkt
                auto curve = canvas->getSelectedCurve();
                if (curve && pointIndex >= 0 && pointIndex < static_cast<int>(curve->getPointCount())) {
                    canvas->startMovingControlPoints();
                    movePointModeAction->setChecked(true);
                    canvas->selectControlPoint(pointIndex);
                    updateModeLabel();
                    statusBar->showMessage(QString("Edytowanie punktu kontrolnego %1. Możesz go przesunąć na canvasie.").arg(std::to_string(pointIndex)));
                }
            }
        }
    });

    // Inicjalne wypełnienie drzewa obiektów
    updateObjectsTree();

    // Wyświetlenie okna
    mainWindow.show();
    statusBar->showMessage("Aplikacja gotowa do użycia");

    // Uruchomienie pętli zdarzeń aplikacji
    return app.exec();
}
