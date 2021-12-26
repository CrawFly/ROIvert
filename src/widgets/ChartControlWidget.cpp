#include "widgets/ChartControlWidget.h"

#include <QBoxLayout>
#include <QSpinBox>

struct ChartControlWidget::pimpl {
    QHBoxLayout toplay;
    QSpinBox spinHeight;
};

ChartControlWidget::ChartControlWidget(QWidget* parent) : QWidget(parent), impl(std::make_unique<pimpl>()) {
    this->setLayout(&impl->toplay);
    impl->spinHeight.setFixedWidth(100);
    // need an auto set on minimum...

    impl->toplay.addWidget(&impl->spinHeight);
    impl->toplay.addStretch(1);

    connect(&impl->spinHeight, QOverload<int>::of(&QSpinBox::valueChanged), [&](int val) { emit heightChanged(val); });
}

ChartControlWidget::~ChartControlWidget() {
}

void ChartControlWidget::changeMinimumHeight(int minheight) {
    impl->spinHeight.setMinimum(minheight);
}

void ChartControlWidget::changeHeight(int newheight) {
    impl->spinHeight.setValue(newheight);
}