#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <memory>
#include <QCheckBox>

#include "../Geometry/BezierCurve.h"

class WeightsPanel : public QWidget {
    Q_OBJECT

public:
    explicit WeightsPanel(QWidget* parent = nullptr);
    ~WeightsPanel() override = default;

    void setBezierCurve(std::shared_ptr<BezierCurve> curve);
    void updateWeights();
    void clear();
    void setShowWeights(bool show) const;

    signals:
        void weightChanged(int pointIndex, double weight);
    void weightsReset();
    void weightsNormalized();
    void showWeightsToggled(bool show);

    private slots:
    void onWeightChanged();
    void onResetWeights();
    void onNormalizeWeights();

private:
    void createLayout();
    void updateWeightControls();
    void connectSignals();

    std::shared_ptr<BezierCurve> m_curve;
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_weightsWidget;
    QVBoxLayout* m_weightsLayout;

    QPushButton* m_resetButton;
    QPushButton* m_normalizeButton;
    QCheckBox* m_showWeightsCheckBox;

    std::vector<QDoubleSpinBox*> m_weightSpinBoxes;
    std::vector<QLabel*> m_pointLabels;
};
