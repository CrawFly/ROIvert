// todo: when the chart style changes chartcontrolwidget neeeds to re-evaluate its minimum height

#include "widgets/ChartControlWidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include <QPushButton>

struct ChartControlWidget::pimpl {
    QHBoxLayout toplay;
    QSpinBox spinHeight;
    QDoubleSpinBox spinTMin;
    QDoubleSpinBox spinTMax;
    QPushButton resetTimeRange;
    
};

ChartControlWidget::ChartControlWidget(QWidget* parent) : QWidget(parent), impl(std::make_unique<pimpl>()) {
    this->setLayout(&impl->toplay);
    impl->spinHeight.setFixedWidth(100);
    impl->spinHeight.setMaximum(999999);
    impl->spinHeight.setMinimum(50);
    impl->spinHeight.setToolTip(tr("Height of the line charts in pixels."));

    impl->toplay.addWidget(new QLabel(tr("Line Chart Height:")));
    impl->toplay.addWidget(&impl->spinHeight);
    
    impl->toplay.addStretch(1);
    
    impl->toplay.addWidget(new QLabel(tr("Time Range:")));
    impl->toplay.addWidget(&impl->spinTMin);
    impl->toplay.addWidget(new QLabel("-"));
    impl->toplay.addWidget(&impl->spinTMax);

    impl->resetTimeRange.setText(tr("Reset"));
    impl->toplay.addWidget(&impl->resetTimeRange);

    connect(&impl->spinHeight, QOverload<int>::of(&QSpinBox::valueChanged), [&](int val) { emit lineChartHeightChanged(val); });
}

ChartControlWidget::~ChartControlWidget() {
}

void ChartControlWidget::changeMinimumLineChartHeight(int minheight) {
    impl->spinHeight.setMinimum(minheight);
}

void ChartControlWidget::changeLineChartHeight(int newheight) {
    impl->spinHeight.setValue(newheight);
}