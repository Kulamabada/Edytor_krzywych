#include "WeightsPanel.h"

#include <iostream>


WeightsPanel::WeightsPanel(QWidget* parent)
    : QWidget(parent)
    , m_curve(nullptr)
    , m_mainLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_weightsWidget(nullptr)
    , m_weightsLayout(nullptr)
{
    createLayout();
    connectSignals();
}

void WeightsPanel::createLayout() {
    m_mainLayout = new QVBoxLayout(this);

    m_showWeightsCheckBox = new QCheckBox("Pokaż wagi");
    m_showWeightsCheckBox->setChecked(false);
    m_mainLayout->addWidget(m_showWeightsCheckBox);

    auto* titleLabel = new QLabel("Wagi punktów kontrolnych");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    m_mainLayout->addWidget(titleLabel);

    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_weightsWidget = new QWidget;
    m_weightsLayout = new QVBoxLayout(m_weightsWidget);
    m_scrollArea->setWidget(m_weightsWidget);
    m_mainLayout->addWidget(m_scrollArea);

    auto* controlsGroup = new QGroupBox("Operacje na wagach");
    auto* controlsLayout = new QVBoxLayout(controlsGroup);

    m_resetButton = new QPushButton("Resetuj (1.0)");
    m_normalizeButton = new QPushButton("Normalizuj");

    controlsLayout->addWidget(m_resetButton);
    controlsLayout->addWidget(m_normalizeButton);

    m_mainLayout->addWidget(controlsGroup);
    m_mainLayout->addStretch();

    setEnabled(false);
}

void WeightsPanel::connectSignals() {
    connect(m_resetButton, &QPushButton::clicked, this, &WeightsPanel::onResetWeights);
    connect(m_normalizeButton, &QPushButton::clicked, this, &WeightsPanel::onNormalizeWeights);
    connect(m_showWeightsCheckBox, &QCheckBox::toggled, this, &WeightsPanel::showWeightsToggled);
}

void WeightsPanel::setBezierCurve(std::shared_ptr<BezierCurve> curve) {
    m_curve = curve;
    updateWeights();
    setEnabled(curve != nullptr);
}

void WeightsPanel::updateWeights() {
    if (!m_curve) {
        clear();
        return;
    }

    updateWeightControls();
}

void WeightsPanel::updateWeightControls() {
    for (auto* spinBox : m_weightSpinBoxes)
        spinBox->deleteLater();

    for (auto* label : m_pointLabels)
        label->deleteLater();


    m_weightSpinBoxes.clear();
    m_pointLabels.clear();

    if (!m_curve || !m_curve->hasControlPoints()) return;

    size_t pointCount = m_curve->getControlPointCount();

    for (size_t i = 0; i < pointCount; ++i) {
        auto* layout = new QHBoxLayout;

        auto* label = new QLabel(QString("P%1:").arg(std::to_string(i)));
        label->setFixedWidth(30);
        m_pointLabels.push_back(label);
        layout->addWidget(label);

        auto* spinBox = new QDoubleSpinBox;
        spinBox->setRange(0.01, 100.0);
        spinBox->setValue(m_curve->getWeight(i));
        spinBox->setDecimals(3);
        spinBox->setSingleStep(0.1);
        spinBox->setProperty("pointIndex", static_cast<int>(i));

        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &WeightsPanel::onWeightChanged);

        m_weightSpinBoxes.push_back(spinBox);
        layout->addWidget(spinBox);

        m_weightsLayout->addLayout(layout);
    }
}

void WeightsPanel::clear() {
    for (auto* spinBox : m_weightSpinBoxes)
        spinBox->deleteLater();

    for (auto* label : m_pointLabels)
        label->deleteLater();

    m_weightSpinBoxes.clear();
    m_pointLabels.clear();
}

void WeightsPanel::onWeightChanged() {
    auto* spinBox = qobject_cast<QDoubleSpinBox*>(sender());
    if (!spinBox || !m_curve) return;

    int pointIndex = spinBox->property("pointIndex").toInt();
    double weight = spinBox->value();

    emit weightChanged(pointIndex, weight);
}

void WeightsPanel::onResetWeights() {
    if (m_curve) emit weightsReset();
}

void WeightsPanel::onNormalizeWeights() {
    if (m_curve) emit weightsNormalized();
}

void WeightsPanel::setShowWeights(const bool show) const {
    m_showWeightsCheckBox->setChecked(show);
}
