#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QTimer>
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
#include <QToolButton>
#include <QComboBox>
#include <QPushButton>
#include <QColorDialog>
#include <QDoubleSpinBox>

#include "Graphics/Canvas.h"
#include "Core/CommandManager.h"
#include "Geometry/InterpolatedCurve.h"
#include "Geometry/Point.h"
#include "Graphics/QtRenderer.h"
#include "Math/EigenMathAdapter.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Edytor Krzywych");
    mainWindow.resize(1024, 768);

    QMenuBar *menuBar = mainWindow.menuBar();

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

    QMenu *editMenu = menuBar->addMenu("Edycja");
    QAction *undoAction = editMenu->addAction(QIcon::fromTheme("edit-undo"), "Cofnij");
    undoAction->setShortcut(QKeySequence::Undo);
    QAction *redoAction = editMenu->addAction(QIcon::fromTheme("edit-redo"), "Ponów");
    redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addSeparator();
    QAction *deleteAction = editMenu->addAction(QIcon::fromTheme("edit-delete"), "Usuń");
    deleteAction->setShortcut(QKeySequence::Delete);

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

    auto *editModeGroup = new QActionGroup(&mainWindow);
    editModeGroup->addAction(selectModeAction);
    editModeGroup->addAction(addPointModeAction);
    editModeGroup->addAction(movePointModeAction);
    selectModeAction->setChecked(true);

    auto *viewMenu = menuBar->addMenu("Widok");
    QAction *zoomInAction = viewMenu->addAction(QIcon::fromTheme("zoom-in"), "Przybliż");
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    QAction *zoomOutAction = viewMenu->addAction(QIcon::fromTheme("zoom-out"), "Oddal");
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    QAction *resetViewAction = viewMenu->addAction(QIcon::fromTheme("zoom-original"), "Resetuj widok");
    resetViewAction->setShortcut(QKeySequence("Ctrl+0"));
    QAction *fitContentAction = viewMenu->addAction(QIcon::fromTheme("zoom-fit-best"), "Dopasuj do zawartości");

    QMenu *curvesMenu = menuBar->addMenu("Krzywe");
    // QAction *bezierAction = curvesMenu->addAction("Krzywa Beziera");
    QMenu *interpolatedCurvesMenu = curvesMenu->addMenu("Krzywa interpolacyjna");
    QAction *nifsAction = interpolatedCurvesMenu->addAction("NIFS3 (Natural Cubic Spline)");
    QAction *oifsAction = interpolatedCurvesMenu->addAction("OIFS3 (Closed Cubic Spline)");
    QAction *lagrangeAction = interpolatedCurvesMenu->addAction("Lagrange");
    QAction *barycentricAction = interpolatedCurvesMenu->addAction("Barycentric");
    QAction *parametricAction = curvesMenu->addAction("Krzywa parametryczna");

    auto interpolatedButton = new QToolButton();
    interpolatedButton->setText("Krzywa interpolacyjna");
    interpolatedButton->setPopupMode(QToolButton::InstantPopup);
    interpolatedButton->setMenu(interpolatedCurvesMenu);

    QMenu *helpMenu = menuBar->addMenu("Pomoc");
    QAction *aboutAction = helpMenu->addAction(QIcon::fromTheme("help-about"), "O programie");

    auto *toolBar = new QToolBar("Narzędzia główne", &mainWindow);
    mainWindow.addToolBar(toolBar);

    toolBar->addAction(newAction);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addSeparator();
    toolBar->addAction(undoAction);
    toolBar->addAction(redoAction);
    toolBar->addSeparator();
    // toolBar->addAction(bezierAction);
    toolBar->addWidget(interpolatedButton);
    toolBar->addAction(parametricAction);
    toolBar->addSeparator();
    toolBar->addAction(zoomInAction);
    toolBar->addAction(zoomOutAction);
    toolBar->addAction(resetViewAction);

    auto *editToolBar = new QToolBar("Narzędzia edycji punktów", &mainWindow);
    mainWindow.addToolBar(Qt::TopToolBarArea, editToolBar);

    editToolBar->addAction(selectModeAction);
    editToolBar->addAction(addPointModeAction);
    editToolBar->addAction(movePointModeAction);
    editToolBar->addSeparator();
    editToolBar->addAction(deletePointAction);
    editToolBar->addAction(finishEditingAction);

    auto *centralWidget = new QWidget(&mainWindow);
    auto *layout = new QVBoxLayout(centralWidget);

    Canvas *canvas = new Canvas(centralWidget);
    layout->addWidget(canvas);

    auto renderer = std::make_shared<QtRenderer>();
    canvas->setRenderer(renderer);

    mainWindow.setCentralWidget(centralWidget);

    QDockWidget *objectsDock = new QDockWidget("Obiekty i punkty kontrolne", &mainWindow);
    objectsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QTreeWidget *objectsTree = new QTreeWidget(objectsDock);
    objectsTree->setHeaderLabels(QStringList() << "Nazwa" << "Współrzędne");
    objectsTree->setColumnCount(2);
    objectsTree->setRootIsDecorated(true);
    objectsTree->setExpandsOnDoubleClick(true);
    objectsDock->setWidget(objectsTree);

    mainWindow.addDockWidget(Qt::RightDockWidgetArea, objectsDock);

    QDockWidget *propertiesDock = new QDockWidget("Właściwości", &mainWindow);
    propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *propertiesWidget = new QWidget(propertiesDock);
    QVBoxLayout *propertiesLayout = new QVBoxLayout(propertiesWidget);
    propertiesLayout->addWidget(new QLabel("Wybierz obiekt, aby edytować jego właściwości"));
    propertiesDock->setWidget(propertiesWidget);

    mainWindow.addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

    QStatusBar *statusBar = mainWindow.statusBar();
    QLabel *modeLabel = new QLabel("Tryb: Normalny");
    statusBar->addPermanentWidget(modeLabel);
    statusBar->showMessage("Gotowy");

    auto mathProvider = std::make_shared<EigenMathAdapter>();

    CommandManager commandManager;

    auto updateObjectsTree = [objectsTree, canvas]() {
        objectsTree->clear();

        int index = 0;
        for (const auto& obj : canvas->objects()) {
            QTreeWidgetItem* item = new QTreeWidgetItem(objectsTree);
            item->setText(0, QString("Obiekt %1").arg(std::to_string(index + 1)));

            auto curve = std::dynamic_pointer_cast<InterpolatedCurve>(obj);
            if (curve) {
                item->setText(1, "Krzywa");

                const auto& points = curve->getPoints();
                for (size_t i = 0; i < points.size(); ++i) {
                    QTreeWidgetItem* pointItem = new QTreeWidgetItem(item);
                    const Point& p = points[i];
                    pointItem->setText(0, QString("Punkt %1").arg(std::to_string(i)));
                    pointItem->setText(1, QString("(%1, %2)")
                        .arg(QString::number(p.x(), 'f', 2), QString::number(p.y(), 'f', 2)));

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

    std::function<void()> updatePropertiesPanel = [&]() {
    QLayoutItem* child;
        while ((child = propertiesLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }

        auto selectedObject = canvas->selectedObject();
        if (!selectedObject) {
            propertiesLayout->addWidget(new QLabel("Wybierz obiekt, aby edytować jego właściwości"));
            return;
        }

        auto curve = std::dynamic_pointer_cast<InterpolatedCurve>(selectedObject);
        if (curve) {
            propertiesLayout->addWidget(new QLabel("<b>Właściwości krzywej interpolowanej</b>"));

            propertiesLayout->addWidget(new QLabel("Typ interpolacji:"));

            QComboBox* typeCombo = new QComboBox();
            typeCombo->addItem("NIFS3 (Natural Cubic Spline)", QVariant::fromValue(static_cast<int>(InterpolatedCurve::InterpolationType::NIFS3)));
            typeCombo->addItem("OIFS3 (Closed Cubic Spline)", QVariant::fromValue(static_cast<int>(InterpolatedCurve::InterpolationType::OIFS3)));
            typeCombo->addItem("Lagrange", QVariant::fromValue(static_cast<int>(InterpolatedCurve::InterpolationType::Lagrange)));
            typeCombo->addItem("Barycentric", QVariant::fromValue(static_cast<int>(InterpolatedCurve::InterpolationType::Barycentric)));

            typeCombo->setCurrentIndex(static_cast<int>(curve->getInterpolationType()));
            propertiesLayout->addWidget(typeCombo);

            QObject::connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [curve, statusBar, canvas, &updatePropertiesPanel](int index) {
                    const auto type = static_cast<InterpolatedCurve::InterpolationType>(index);
                    curve->setInterpolationType(type);
                    canvas->refresh();
                    QTimer::singleShot(0, [&updatePropertiesPanel]() {
                            updatePropertiesPanel();
                        });

                    QString typeName;
                    switch (type) {
                        case InterpolatedCurve::InterpolationType::NIFS3: typeName = "NIFS3"; break;
                        case InterpolatedCurve::InterpolationType::OIFS3: typeName = "OIFS3"; break;
                        case InterpolatedCurve::InterpolationType::Lagrange: typeName = "Lagrange"; break;
                        case InterpolatedCurve::InterpolationType::Barycentric: typeName = "Barycentric"; break;
                    }

                    statusBar->showMessage(QString("Zmieniono typ interpolacji na %1").arg(typeName));
                });

            // Kolor linii - nowa sekcja
            propertiesLayout->addWidget(new QLabel("Kolor linii:"));

            QPushButton* colorButton = new QPushButton();
            colorButton->setFixedSize(80, 25);
            QColor currentColor = curve->getColor();
            QString styleSheet = QString("background-color: %1;").arg(currentColor.name());
            colorButton->setStyleSheet(styleSheet);

            propertiesLayout->addWidget(colorButton);

            QObject::connect(colorButton, &QPushButton::clicked, [curve, colorButton, canvas, statusBar]() {
                QColor color = QColorDialog::getColor(curve->getColor(), nullptr, "Wybierz kolor linii");
                if (color.isValid()) {
                    curve->setColor(color);
                    QString styleSheet = QString("background-color: %1;").arg(color.name());
                    colorButton->setStyleSheet(styleSheet);
                    canvas->refresh();
                    statusBar->showMessage("Zmieniono kolor linii");
                }
            });

            propertiesLayout->addWidget(new QLabel("Grubość linii:"));

            QDoubleSpinBox* widthSpinBox = new QDoubleSpinBox();
            widthSpinBox->setRange(0.1, 10.0);
            widthSpinBox->setSingleStep(0.1);
            widthSpinBox->setValue(curve->getLineWidth());

            propertiesLayout->addWidget(widthSpinBox);

            QObject::connect(widthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [curve, canvas, statusBar](double value) {
                curve->setLineWidth(value);
                canvas->refresh();
                statusBar->showMessage(QString("Zmieniono grubość linii na %1").arg(std::to_string(value)));
            });

            propertiesLayout->addWidget(new QLabel("Styl linii:"));

            QComboBox* lineStyleCombo = new QComboBox();
            lineStyleCombo->addItem("Ciągła", static_cast<int>(Qt::SolidLine));
            lineStyleCombo->addItem("Przerywana", static_cast<int>(Qt::DashLine));
            lineStyleCombo->addItem("Kropkowana", static_cast<int>(Qt::DotLine));
            lineStyleCombo->addItem("Kreska-kropka", static_cast<int>(Qt::DashDotLine));
            lineStyleCombo->addItem("Kreska-kropka-kropka", static_cast<int>(Qt::DashDotDotLine));

            int styleIndex = 0;
            switch (curve->getLineStyle()) {
                case Qt::SolidLine: styleIndex = 0; break;
                case Qt::DashLine: styleIndex = 1; break;
                case Qt::DotLine: styleIndex = 2; break;
                case Qt::DashDotLine: styleIndex = 3; break;
                case Qt::DashDotDotLine: styleIndex = 4; break;
                default: styleIndex = 0; break;
            }

            lineStyleCombo->setCurrentIndex(styleIndex);

            propertiesLayout->addWidget(lineStyleCombo);

            QObject::connect(lineStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [curve, lineStyleCombo, canvas, statusBar](int index) {
                Qt::PenStyle style = static_cast<Qt::PenStyle>(lineStyleCombo->itemData(index).toInt());
                curve->setLineStyle(style);
                canvas->refresh();
                statusBar->showMessage("Zmieniono styl linii");
            });

            propertiesLayout->addWidget(new QLabel(QString("Długość krzywej: %1")
                .arg(QString::number(curve->getLength(), 'f', 2))));
        } else {
            propertiesLayout->addWidget(new QLabel("Wybierz krzywą interpolowaną, aby edytować jej właściwości"));
        }
        propertiesLayout->addStretch();
        };

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

    auto createInterpolatedCurve = [&](InterpolatedCurve::InterpolationType type, const QString &typeName) {
        auto curve = std::make_shared<InterpolatedCurve>(mathProvider, type);

        // Ustawiamy domyślne wartości stylu
        curve->setColor(Qt::blue);
        curve->setLineWidth(1.0);
        curve->setLineStyle(Qt::SolidLine);

        curve->addPoint(Point(0, 0));
        curve->addPoint(Point(50, 100));
        curve->addPoint(Point(100, 0));
        canvas->addObject(curve);
        canvas->selectObject(curve);
        canvas->refresh();
        updateObjectsTree();
        statusBar->showMessage(QString("Utworzono krzywą interpolowaną typu %1").arg(typeName));
    };

    QObject::connect(nifsAction, &QAction::triggered, [&]() {
        createInterpolatedCurve(InterpolatedCurve::InterpolationType::NIFS3, "NIFS3");
    });

    QObject::connect(oifsAction, &QAction::triggered, [&]() {
        createInterpolatedCurve(InterpolatedCurve::InterpolationType::OIFS3, "OIFS3");
    });

    QObject::connect(lagrangeAction, &QAction::triggered, [&]() {
        createInterpolatedCurve(InterpolatedCurve::InterpolationType::Lagrange, "Lagrange");
    });

    QObject::connect(barycentricAction, &QAction::triggered, [&]() {
        createInterpolatedCurve(InterpolatedCurve::InterpolationType::Barycentric, "Barycentric");
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
        if (auto curve = canvas->getSelectedCurve()) {
            QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
            QApplication::sendEvent(canvas, &keyEvent);

            updateObjectsTree();
            updatePropertiesPanel();
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
        updatePropertiesPanel();
        auto objects = canvas->objects();
        auto it = std::find(objects.begin(), objects.end(), obj);
        if (it != objects.end()) {
            int index = static_cast<int>(std::distance(objects.begin(), it));

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
        updatePropertiesPanel();
        statusBar->showMessage("Anulowano zaznaczenie");
    });

    QObject::connect(canvas, &Canvas::controlPointAdded, [&](int pointIndex) {
        updateObjectsTree();
        updatePropertiesPanel();
        statusBar->showMessage(QString("Dodano punkt kontrolny %1").arg(std::to_string(pointIndex)));
    });

    QObject::connect(canvas, &Canvas::controlPointMoved, [&](int pointIndex) {
        updateObjectsTree();
        updatePropertiesPanel();
        statusBar->showMessage(QString("Przesunięto punkt kontrolny %1").arg(std::to_string(pointIndex)));
    });

    QObject::connect(canvas, &Canvas::controlPointSelected, [&](int pointIndex) {
        updateObjectsTree();

        auto selectedObj = canvas->selectedObject();
        auto objects = canvas->objects();
        auto it = std::find(objects.begin(), objects.end(), selectedObj);

        if (it != objects.end()) {
            int objectIndex = static_cast<int>(std::distance(objects.begin(), it));

            for (int i = 0; i < objectsTree->topLevelItemCount(); ++i) {
                QTreeWidgetItem* objectItem = objectsTree->topLevelItem(i);
                if (objectItem->data(0, Qt::UserRole).toInt() == objectIndex) {
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

    QObject::connect(objectsTree, &QTreeWidget::itemClicked, [&](QTreeWidgetItem* item, int column) {
        if (item->parent() == nullptr) {
            int index = item->data(0, Qt::UserRole).toInt();
            if (index >= 0 && index < static_cast<int>(canvas->objects().size())) {
                canvas->selectObject(canvas->objects()[index]);
                canvas->finishEditing();
                selectModeAction->setChecked(true);
                updateModeLabel();
            }
        }
        else {
            int pointIndex = item->data(0, Qt::UserRole).toInt();
            int objectIndex = item->data(0, Qt::UserRole + 1).toInt();

            if (objectIndex >= 0 && objectIndex < static_cast<int>(canvas->objects().size())) {
                canvas->selectObject(canvas->objects()[objectIndex]);

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

    QObject::connect(objectsTree, &QTreeWidget::itemDoubleClicked, [&](QTreeWidgetItem* item, int column) {
        if (item->parent() != nullptr) {
            int pointIndex = item->data(0, Qt::UserRole).toInt();
            int objectIndex = item->data(0, Qt::UserRole + 1).toInt();

            if (objectIndex >= 0 && objectIndex < static_cast<int>(canvas->objects().size())) {
                canvas->selectObject(canvas->objects()[objectIndex]);

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

    updateObjectsTree();

    mainWindow.show();
    statusBar->showMessage("Aplikacja gotowa do użycia");

    return app.exec();
}
