#include "MainWindow.h"
#include "ActionHandler.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_canvas(nullptr)
    , m_commandManager(std::make_unique<CommandManager>())
    , m_mathProvider(std::make_shared<EigenMathAdapter>())
    , m_renderer(std::make_shared<QtRenderer>())
    , m_objectsTree(nullptr)
    , m_modeLabel(nullptr)
{
    setWindowTitle("Edytor Krzywych");
    createMenus();
    createToolbars();
    setupCentralWidget();
    createDockWidgets();
    connectSignals();

    m_actionHandler = std::make_unique<ActionHandler>(*this);
    showMaximized();
    statusBar()->showMessage("Aplikacja gotowa do użycia");
}

MainWindow::~MainWindow() = default;

void MainWindow::createMenus() {
    m_fileMenu = menuBar()->addMenu("Plik");
    m_editMenu = menuBar()->addMenu("Edycja");
    m_viewMenu = menuBar()->addMenu("Widok");
    m_curvesMenu = menuBar()->addMenu("Krzywe");
    m_imageMenu = menuBar()->addMenu("Obrazy");
    m_helpMenu = menuBar()->addMenu("Pomoc");

    m_newAction = m_fileMenu->addAction(QIcon::fromTheme("document-new"), "Nowy");
    m_newAction->setShortcut(QKeySequence::New);
    m_openAction = m_fileMenu->addAction(QIcon::fromTheme("document-open"), "Otwórz...");
    m_openAction->setShortcut(QKeySequence::Open);
    m_saveAction = m_fileMenu->addAction(QIcon::fromTheme("document-save"), "Zapisz");
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAsAction = m_fileMenu->addAction(QIcon::fromTheme("document-save-as"), "Zapisz jako...");
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_fileMenu->addSeparator();
    m_exitAction = m_fileMenu->addAction(QIcon::fromTheme("application-exit"), "Wyjście");
    m_exitAction->setShortcut(QKeySequence::Quit);

    m_undoAction = m_editMenu->addAction(QIcon::fromTheme("edit-undo"), "Cofnij");
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_redoAction = m_editMenu->addAction(QIcon::fromTheme("edit-redo"), "Ponów");
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_editMenu->addSeparator();
    m_deleteAction = m_editMenu->addAction(QIcon::fromTheme("edit-delete"), "Usuń");

    m_editMenu->addSeparator();
    m_controlPointsMenu = m_editMenu->addMenu("Edycja punktów kontrolnych");

    m_selectModeAction = m_controlPointsMenu->addAction(QIcon::fromTheme("edit-select"), "Zaznacz punkt");
    m_selectModeAction->setCheckable(true);

    m_addPointModeAction = m_controlPointsMenu->addAction(QIcon::fromTheme("list-add"), "Dodaj punkt");
    m_addPointModeAction->setCheckable(true);

    m_movePointModeAction = m_controlPointsMenu->addAction(QIcon::fromTheme("transform-move"), "Przesuń punkt");
    m_movePointModeAction->setCheckable(true);

    m_deletePointAction = m_controlPointsMenu->addAction(QIcon::fromTheme("edit-delete"), "Usuń punkt");

    m_finishEditingAction = m_controlPointsMenu->addAction(QIcon::fromTheme("dialog-cancel"), "Zakończ edycję");
    m_finishEditingAction->setShortcut(QKeySequence("Esc"));

    m_editModeGroup = new QActionGroup(this);
    m_editModeGroup->addAction(m_selectModeAction);
    m_editModeGroup->addAction(m_addPointModeAction);
    m_editModeGroup->addAction(m_movePointModeAction);
    m_selectModeAction->setChecked(true);

    m_zoomInAction = m_viewMenu->addAction(QIcon::fromTheme("zoom-in"), "Przybliż");
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    m_zoomOutAction = m_viewMenu->addAction(QIcon::fromTheme("zoom-out"), "Oddal");
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    m_resetViewAction = m_viewMenu->addAction(QIcon::fromTheme("zoom-original"), "Resetuj widok");
    m_resetViewAction->setShortcut(QKeySequence("Ctrl+0"));
    m_fitContentAction = m_viewMenu->addAction(QIcon::fromTheme("zoom-fit-best"), "Dopasuj do zawartości");

    m_interpolatedCurvesMenu = m_curvesMenu->addMenu("Krzywa interpolacyjna");
    m_nifsAction = m_interpolatedCurvesMenu->addAction("NIFS3");
    m_oifsAction = m_interpolatedCurvesMenu->addAction("OIFS3");
    m_lagrangeAction = m_interpolatedCurvesMenu->addAction("Lagrange");
    m_barycentricAction = m_interpolatedCurvesMenu->addAction("Barycentric");
    m_bezierAction = m_curvesMenu->addAction("Krzywa Beziera");

    m_addImageAction = m_imageMenu->addAction("Dodaj obraz...");
    m_editImageAction = m_imageMenu->addAction("Edytuj narożniki obrazu");
    m_lockImageAction = m_imageMenu->addAction("Zablokuj/Odblokuj obraz");

    m_aboutAction = m_helpMenu->addAction(QIcon::fromTheme("help-about"), "O programie");
}

void MainWindow::createToolbars() {
    m_mainToolBar = new QToolBar("Narzędzia główne", this);
    addToolBar(m_mainToolBar);

    m_mainToolBar->addAction(m_newAction);
    m_mainToolBar->addAction(m_openAction);
    m_mainToolBar->addAction(m_saveAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_undoAction);
    m_mainToolBar->addAction(m_redoAction);
    m_mainToolBar->addSeparator();

    m_interpolatedButton = new QToolButton();
    m_interpolatedButton->setText("Krzywa interpolacyjna");
    m_interpolatedButton->setPopupMode(QToolButton::InstantPopup);
    m_interpolatedButton->setMenu(m_interpolatedCurvesMenu);
    m_mainToolBar->addWidget(m_interpolatedButton);

    m_mainToolBar->addAction(m_bezierAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_zoomInAction);
    m_mainToolBar->addAction(m_zoomOutAction);
    m_mainToolBar->addAction(m_resetViewAction);

    m_editToolBar = new QToolBar("Narzędzia edycji punktów", this);
    addToolBar(Qt::TopToolBarArea, m_editToolBar);

    m_editToolBar->addAction(m_selectModeAction);
    m_editToolBar->addAction(m_addPointModeAction);
    m_editToolBar->addAction(m_movePointModeAction);
    m_editToolBar->addSeparator();
    m_editToolBar->addAction(m_deletePointAction);
    m_editToolBar->addAction(m_finishEditingAction);

    m_imageToolBar = new QToolBar("Obrazy", this);
    addToolBar(m_imageToolBar);

    m_imageToolBar->addAction(m_addImageAction);
    m_imageToolBar->addAction(m_editImageAction);
    m_imageToolBar->addAction(m_lockImageAction);
}

void MainWindow::setupCentralWidget() {
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    m_canvas = new Canvas(centralWidget);
    layout->addWidget(m_canvas);

    m_canvas->setRenderer(m_renderer);

    setCentralWidget(centralWidget);
}

void MainWindow::createDockWidgets() {
    m_objectsDock = new QDockWidget("Obiekty i punkty kontrolne", this);
    m_objectsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    m_objectsTree = new QTreeWidget(m_objectsDock);
    m_objectsTree->setHeaderLabels(QStringList() << "Nazwa" << "Współrzędne");
    m_objectsTree->setColumnCount(2);
    m_objectsTree->setRootIsDecorated(true);
    m_objectsTree->setExpandsOnDoubleClick(true);
    m_objectsDock->setWidget(m_objectsTree);

    addDockWidget(Qt::LeftDockWidgetArea, m_objectsDock);

    m_propertiesDock = new QDockWidget("Właściwości", this);
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    m_propertiesWidget = new QWidget(m_propertiesDock);
    m_propertiesLayout = new QVBoxLayout(m_propertiesWidget);
    m_propertiesLayout->addWidget(new QLabel("Wybierz obiekt, aby edytować jego właściwości"));
    m_propertiesDock->setWidget(m_propertiesWidget);

    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);

    m_modeLabel = new QLabel("Tryb: Normalny");
    statusBar()->addPermanentWidget(m_modeLabel);
}

void MainWindow::connectSignals() {
    connect(m_canvas, &Canvas::objectAdded, this, &MainWindow::onObjectAdded);
    connect(m_canvas, &Canvas::objectRemoved, this, &MainWindow::onObjectRemoved);
    connect(m_canvas, &Canvas::objectSelected, this, &MainWindow::onObjectSelected);
    connect(m_canvas, &Canvas::selectionCleared, this, &MainWindow::onSelectionCleared);
    connect(m_canvas, &Canvas::controlPointAdded, this, &MainWindow::onControlPointAdded);
    connect(m_canvas, &Canvas::controlPointMoved, this, &MainWindow::onControlPointMoved);
    connect(m_canvas, &Canvas::controlPointSelected, this, &MainWindow::onControlPointSelected);
    connect(m_canvas, &Canvas::editingStarted, this, &MainWindow::onEditingStarted);
    connect(m_canvas, &Canvas::editingFinished, this, &MainWindow::onEditingFinished);

    connect(m_objectsTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int column) {
        if (item->parent() == nullptr) {
            int index = item->data(0, Qt::UserRole).toInt();
            if (index >= 0 && index < static_cast<int>(m_canvas->objects().size())) {
                m_canvas->selectObject(m_canvas->objects()[index]);
                m_canvas->finishEditing();
                m_selectModeAction->setChecked(true);
                updateModeLabel();
            }
        }
        else {
            int pointIndex = item->data(0, Qt::UserRole).toInt();
            int objectIndex = item->data(0, Qt::UserRole + 1).toInt();
            if (objectIndex >= 0 && objectIndex < static_cast<int>(m_canvas->objects().size())) {
                m_canvas->selectObject(m_canvas->objects()[objectIndex]);

                auto curve = std::dynamic_pointer_cast<Curve>(m_canvas->selectedObject());
                if (curve && pointIndex >= 0 && pointIndex < static_cast<int>(curve->getControlPointCount())) {
                    if (m_canvas->getEditMode() == Canvas::EditMode::None) {
                        m_canvas->setEditMode(Canvas::EditMode::SelectPoint);
                        m_selectModeAction->setChecked(true);
                    }
                    m_canvas->selectControlPoint(pointIndex);
                    updateModeLabel();
                }
            }
        }
    });
}

void MainWindow::updateObjectsTree() {
    m_objectsTree->clear();

    int index = 0;
    for (const auto& obj : m_canvas->objects()) {
        auto* item = new QTreeWidgetItem(m_objectsTree);
        item->setText(0, QString("Obiekt %1").arg(std::to_string(index + 1)));

        if (const auto curve = std::dynamic_pointer_cast<InterpolatedCurve>(obj)) {
            item->setText(1, "Krzywa");

            const auto& points = curve->getControlPoints();
            for (size_t i = 0; i < points.size(); ++i) {
                auto* pointItem = new QTreeWidgetItem(item);
                const Point& p = points[i];
                pointItem->setText(0, QString("Punkt %1").arg(std::to_string(i)));
                pointItem->setText(1, QString("(%1, %2)")
                    .arg(QString::number(p.x(), 'f', 2), QString::number(p.y(), 'f', 2)));

                pointItem->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<int>(i)));
                pointItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(index));
            }
        }
        else if (const auto image = std::dynamic_pointer_cast<ImageObject>(obj)) {
            item->setText(1, "Obraz");

            const auto& corners = image->getCornerPoints();
            for (size_t i = 0; i < corners.size(); ++i) {
                auto* cornerItem = new QTreeWidgetItem(item);
                const Point& p = corners[i];
                cornerItem->setText(0, QString("Narożnik %1").arg(std::to_string(i)));
                cornerItem->setText(1, QString("(%1, %2)")
                    .arg(QString::number(p.x(), 'f', 2),
                         QString::number(p.y(), 'f', 2)));
            }
        }
        else if (const auto bezierCurve = std::dynamic_pointer_cast<BezierCurve>(obj)) {
            item->setText(1, "Krzywa Béziera");

            const auto& points = bezierCurve->getControlPoints();
            for (size_t i = 0; i < points.size(); ++i) {
                auto* pointItem = new QTreeWidgetItem(item);
                const Point& p = points[i];
                pointItem->setText(0, QString("Punkt %1").arg(std::to_string(i)));
                pointItem->setText(1, QString("(%1, %2)")
                    .arg(QString::number(p.x(), 'f', 2), QString::number(p.y(), 'f', 2)));

                pointItem->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<int>(i)));
                pointItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(index));
            }
        }
        else {
            item->setText(1, "Inny");
        }

        item->setData(0, Qt::UserRole, QVariant::fromValue(index));
        index++;
    }

    m_objectsTree->expandAll();
}

void MainWindow::updateModeLabel() {
    switch (m_canvas->getEditMode()) {
        case Canvas::EditMode::None:
            m_modeLabel->setText("Tryb: Normalny");
            break;
        case Canvas::EditMode::SelectPoint:
            m_modeLabel->setText("Tryb: Zaznaczanie obiektu");
            break;
        case Canvas::EditMode::AddPoint:
            m_modeLabel->setText("Tryb: Dodawanie punktu");
            break;
        case Canvas::EditMode::MovePoint:
            m_modeLabel->setText("Tryb: Przesuwanie punktu");
            break;
    }
}

void MainWindow::onObjectAdded(std::shared_ptr<GeometricObject> obj) {
    updateObjectsTree();
    statusBar()->showMessage("Dodano nowy obiekt");
}

void MainWindow::onObjectRemoved(std::shared_ptr<GeometricObject> obj) {
    updateObjectsTree();
    statusBar()->showMessage("Usunięto obiekt");
}

void MainWindow::onObjectSelected(std::shared_ptr<GeometricObject> obj) {
    updateObjectsTree();
    updatePropertiesPanel();
    auto objects = m_canvas->objects();
    auto it = std::find(objects.begin(), objects.end(), obj);
    if (it != objects.end()) {
        int index = static_cast<int>(std::distance(objects.begin(), it));

        for (int i = 0; i < m_objectsTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = m_objectsTree->topLevelItem(i);
            if (item->data(0, Qt::UserRole).toInt() == index) {
                m_objectsTree->setCurrentItem(item);
                break;
            }
        }
    }

    statusBar()->showMessage("Wybrano obiekt");
}

void MainWindow::onSelectionCleared() {
    m_objectsTree->clearSelection();
    updatePropertiesPanel();
    statusBar()->showMessage("Anulowano zaznaczenie");
}

void MainWindow::onControlPointAdded(const int pointIndex) {
    updateObjectsTree();
    updatePropertiesPanel();
    statusBar()->showMessage(QString("Dodano punkt kontrolny %1").arg(std::to_string(pointIndex)));
}

void MainWindow::onControlPointMoved(int pointIndex) {
    updateObjectsTree();
    updatePropertiesPanel();
    statusBar()->showMessage(QString("Przesunięto punkt kontrolny %1").arg(std::to_string(pointIndex)));
}

void MainWindow::onControlPointSelected(int pointIndex) {
    updateObjectsTree();

    auto selectedObj = m_canvas->selectedObject();
    auto objects = m_canvas->objects();
    auto it = std::find(objects.begin(), objects.end(), selectedObj);
    if (it != objects.end()) {
        int objectIndex = static_cast<int>(std::distance(objects.begin(), it));

        for (int i = 0; i < m_objectsTree->topLevelItemCount(); ++i) {
            const QTreeWidgetItem* objectItem = m_objectsTree->topLevelItem(i);
            if (objectItem->data(0, Qt::UserRole).toInt() == objectIndex) {
                for (int j = 0; j < objectItem->childCount(); ++j) {
                    QTreeWidgetItem* pointItem = objectItem->child(j);
                    if (pointItem->data(0, Qt::UserRole).toInt() == pointIndex) {
                        m_objectsTree->setCurrentItem(pointItem);
                        break;
                    }
                }
                break;
            }
        }
    }

    statusBar()->showMessage(QString("Wybrano punkt kontrolny %1").arg(std::to_string(pointIndex)));
}

void MainWindow::onEditingStarted() {
    updateModeLabel();
    statusBar()->showMessage("Rozpoczęto edycję punktów kontrolnych");
}

void MainWindow::onEditingFinished() {
    m_selectModeAction->setChecked(true);
    updateModeLabel();
    statusBar()->showMessage("Zakończono edycję punktów kontrolnych");
}

void MainWindow::addUnifiedCurveVisibilityControls(std::shared_ptr<Curve> curve) {
    m_propertiesLayout->addWidget(new QLabel("<b>Kontrola wyświetlania:</b>"));

    auto* drawCurveCheckBox = new QCheckBox("Rysuj krzywą główną");
    drawCurveCheckBox->setChecked(curve->getDrawMainCurve());
    m_propertiesLayout->addWidget(drawCurveCheckBox);

    connect(drawCurveCheckBox, &QCheckBox::toggled, [this, curve](bool checked) {
        curve->setDrawMainCurve(checked);
        m_canvas->refresh();
        statusBar()->showMessage(checked ? "Włączono rysowanie krzywej" : "Wyłączono rysowanie krzywej");
    });

    auto* drawPolylineCheckBox = new QCheckBox("Rysuj łamaną łączącą punkty");
    drawPolylineCheckBox->setChecked(curve->getDrawPolyline());
    m_propertiesLayout->addWidget(drawPolylineCheckBox);

    connect(drawPolylineCheckBox, &QCheckBox::toggled, [this, curve](bool checked) {
        curve->setDrawPolyline(checked);
        m_canvas->refresh();
        statusBar()->showMessage(checked ? "Włączono rysowanie łamanej" : "Wyłączono rysowanie łamanej");
    });

    auto* drawConvexHullCheckBox = new QCheckBox("Rysuj otoczkę wypukłą punktów");
    drawConvexHullCheckBox->setChecked(curve->getDrawConvexHull());
    m_propertiesLayout->addWidget(drawConvexHullCheckBox);

    connect(drawConvexHullCheckBox, &QCheckBox::toggled, [this, curve](bool checked) {
        curve->setDrawConvexHull(checked);
        m_canvas->refresh();
        statusBar()->showMessage(checked ? "Włączono rysowanie otoczki wypukłej" : "Wyłączono rysowanie otoczki wypukłej");
    });
}

void MainWindow::addUnifiedCurveStyleControls(std::shared_ptr<Curve> curve) {
    m_propertiesLayout->addWidget(new QLabel("<b>Styl krzywej głównej:</b>"));

    m_propertiesLayout->addWidget(new QLabel("Kolor linii:"));
    QPushButton* colorButton = new QPushButton();
    colorButton->setFixedSize(80, 25);
    QColor currentColor = curve->getColor();
    colorButton->setStyleSheet(QString("background-color: %1;").arg(currentColor.name()));
    m_propertiesLayout->addWidget(colorButton);

    connect(colorButton, &QPushButton::clicked, [this, curve, colorButton]() {
        QColor color = QColorDialog::getColor(curve->getColor(), nullptr, "Wybierz kolor linii");
        if (color.isValid()) {
            curve->setColor(color);
            colorButton->setStyleSheet(QString("background-color: %1;").arg(color.name()));
            m_canvas->refresh();
            statusBar()->showMessage("Zmieniono kolor linii");
        }
    });

    m_propertiesLayout->addWidget(new QLabel("Grubość linii:"));
    auto* widthSpinBox = new QDoubleSpinBox();
    widthSpinBox->setRange(0.1, 10.0);
    widthSpinBox->setSingleStep(0.1);
    widthSpinBox->setValue(curve->getLineWidth());
    m_propertiesLayout->addWidget(widthSpinBox);

    connect(widthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, curve](double value) {
        curve->setLineWidth(value);
        m_canvas->refresh();
        statusBar()->showMessage(QString("Zmieniono grubość linii na %1").arg(QString::number(value)));
    });

    m_propertiesLayout->addWidget(new QLabel("<b>Styl łamanej:</b>"));

    m_propertiesLayout->addWidget(new QLabel("Kolor łamanej:"));
    auto polylineColorButton = new QPushButton();
    polylineColorButton->setFixedSize(80, 25);
    QColor currentPolylineColor = curve->getPolylineColor();
    polylineColorButton->setStyleSheet(QString("background-color: %1;").arg(currentPolylineColor.name()));
    m_propertiesLayout->addWidget(polylineColorButton);

    connect(polylineColorButton, &QPushButton::clicked, [this, curve, polylineColorButton]() {
        QColor color = QColorDialog::getColor(curve->getPolylineColor(), nullptr, "Wybierz kolor łamanej");
        if (color.isValid()) {
            curve->setPolylineColor(color);
            polylineColorButton->setStyleSheet(QString("background-color: %1;").arg(color.name()));
            m_canvas->refresh();
            statusBar()->showMessage("Zmieniono kolor łamanej");
        }
    });

    m_propertiesLayout->addWidget(new QLabel("Grubość łamanej:"));
    auto* polylineWidthSpinBox = new QDoubleSpinBox();
    polylineWidthSpinBox->setRange(0.1, 10.0);
    polylineWidthSpinBox->setSingleStep(0.1);
    polylineWidthSpinBox->setValue(curve->getPolylineWidth());
    m_propertiesLayout->addWidget(polylineWidthSpinBox);

    connect(polylineWidthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, curve](double value) {
        curve->setPolylineWidth(value);
        m_canvas->refresh();
        statusBar()->showMessage(QString("Zmieniono grubość łamanej na %1").arg(QString::number(value)));
    });

    m_propertiesLayout->addWidget(new QLabel("<b>Styl otoczki wypukłej:</b>"));

    m_propertiesLayout->addWidget(new QLabel("Kolor otoczki wypukłej:"));
    QPushButton* convexHullColorButton = new QPushButton();
    convexHullColorButton->setFixedSize(80, 25);
    QColor currentConvexHullColor = curve->getConvexHullColor();
    convexHullColorButton->setStyleSheet(QString("background-color: %1;").arg(currentConvexHullColor.name()));
    m_propertiesLayout->addWidget(convexHullColorButton);

    connect(convexHullColorButton, &QPushButton::clicked, [this, curve, convexHullColorButton]() {
        QColor color = QColorDialog::getColor(curve->getConvexHullColor(), nullptr, "Wybierz kolor otoczki wypukłej");
        if (color.isValid()) {
            curve->setConvexHullColor(color);
            convexHullColorButton->setStyleSheet(QString("background-color: %1;").arg(color.name()));
            m_canvas->refresh();
            statusBar()->showMessage("Zmieniono kolor otoczki wypukłej");
        }
    });

    m_propertiesLayout->addWidget(new QLabel(QString("Długość krzywej: %1")
        .arg(QString::number(curve->getLength(), 'f', 2))));
}

void MainWindow::updatePropertiesPanel() {
    QLayoutItem* child;
    while ((child = m_propertiesLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    auto selectedObject = m_canvas->selectedObject();
    if (!selectedObject) {
        m_propertiesLayout->addWidget(new QLabel("Wybierz obiekt, aby edytować jego właściwości"));
        return;
    }

    if (auto curve = std::dynamic_pointer_cast<InterpolatedCurve>(selectedObject)) {
        m_propertiesLayout->addWidget(new QLabel("<b>Właściwości krzywej interpolowanej</b>"));

        m_propertiesLayout->addWidget(new QLabel("Typ interpolacji:"));
        QComboBox* typeCombo = new QComboBox();
        typeCombo->addItem("NIFS3 (Natural Cubic Spline)", QVariant::fromValue(static_cast<int>(InterpolatedCurve::InterpolationType::NIFS3)));
        typeCombo->addItem("OIFS3 (Closed Cubic Spline)", QVariant::fromValue(static_cast<int>(InterpolatedCurve::InterpolationType::OIFS3)));
        typeCombo->addItem("Lagrange", QVariant::fromValue(static_cast<int>(InterpolatedCurve::InterpolationType::Lagrange)));
        typeCombo->addItem("Barycentric", QVariant::fromValue(static_cast<int>(InterpolatedCurve::InterpolationType::Barycentric)));

        typeCombo->setCurrentIndex(static_cast<int>(curve->getInterpolationType()));
        m_propertiesLayout->addWidget(typeCombo);

        connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, curve](int index) {
                const auto type = static_cast<InterpolatedCurve::InterpolationType>(index);
                curve->setInterpolationType(type);
                m_canvas->refresh();
                QTimer::singleShot(0, [this]() { updatePropertiesPanel(); });

                QString typeName;
                switch (type) {
                    case InterpolatedCurve::InterpolationType::NIFS3: typeName = "NIFS3"; break;
                    case InterpolatedCurve::InterpolationType::OIFS3: typeName = "OIFS3"; break;
                    case InterpolatedCurve::InterpolationType::Lagrange: typeName = "Lagrange"; break;
                    case InterpolatedCurve::InterpolationType::Barycentric: typeName = "Barycentric"; break;
                }
                statusBar()->showMessage(QString("Zmieniono typ interpolacji na %1").arg(typeName));
            });

        addUnifiedCurveVisibilityControls(std::static_pointer_cast<Curve>(curve));
        addUnifiedCurveStyleControls(std::static_pointer_cast<Curve>(curve));
    }
    else if (auto bezierCurve = std::dynamic_pointer_cast<BezierCurve>(selectedObject)) {
        m_propertiesLayout->addWidget(new QLabel("<b>Właściwości krzywej Béziera</b>"));
        m_propertiesLayout->addWidget(new QLabel(QString("Liczba punktów kontrolnych: %1")
                                                .arg(QString::number(bezierCurve->getControlPointCount()))));
        m_propertiesLayout->addWidget(new QLabel(QString("Stopień krzywej: %1")
                                                .arg(QString::number(bezierCurve->getDegree()))));

        addUnifiedCurveVisibilityControls(std::static_pointer_cast<Curve>(bezierCurve));
        addUnifiedCurveStyleControls(std::static_pointer_cast<Curve>(bezierCurve));
    }
    else {
        m_propertiesLayout->addWidget(new QLabel("Wybierz obiekt"));
    }

    m_propertiesLayout->addStretch();
}