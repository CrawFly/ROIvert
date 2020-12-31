#include "ContrastWidget.h"
#include <QGraphicsScene>
#include <QPainterPath>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QBoxLayout>
#include "ContrastWidgetImpl.h"
#include <QDoubleSpinBox>

#include <QDebug>


ContrastWidget::ContrastWidget(QWidget* parent) : QWidget(parent) {
    chart = new ContrastWidgetImpl::ContrastChart;
    spinMin = new QDoubleSpinBox;
    spinGamma = new QDoubleSpinBox;
    spinMax = new QDoubleSpinBox;

    auto lay = new QVBoxLayout(this);
    lay->addWidget(chart);
    chart->setToolTip(tr("A histogram of image data is shown. Slide the minimum/maximum/gamma lines to adjust contrast or set values in the boxes below."));

    auto layEdit = new QHBoxLayout();
    layEdit->addWidget(spinMin);
    layEdit->addStretch(1);
    layEdit->addWidget(spinGamma);
    layEdit->addStretch(1);
    layEdit->addWidget(spinMax);
    lay->addLayout(layEdit);
    lay->setMargin(0);

    spinMin->setSingleStep(.05);
    spinMax->setValue(0.);
    spinMin->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinMin->setToolTip(tr("Minimum: pixels less than this proportion of the pixel range will be shown as black."));

    spinMax->setSingleStep(.05);
    spinMax->setValue(1.);
    spinMax->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinMax->setToolTip(tr("Maximum value: pixels greater than this proportion of the pixel range will be shown as white."));

    spinGamma->setValue(1.);
    spinGamma->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    spinGamma->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinGamma->setToolTip(tr("Gamma correction: power-law nonlinearity applied to pixel brightness."));

    setGammaRange(.001, 10.);

    const auto lamSpin2Chart = [=]() {
        ROIVert::contrast c{ spinMin->value(), spinMax->value(), spinGamma->value() };
        chart->setValues(c);
        emit contrastChanged(c); };

    connect(spinMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), lamSpin2Chart);
    connect(spinMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), lamSpin2Chart);
    connect(spinGamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), lamSpin2Chart);

    const auto lamChart2Spin = [=](ROIVert::contrast c) {
        spinMin->setValue(std::get<0>(c));
        spinMax->setValue(std::get<1>(c));
        spinGamma->setValue(std::get<2>(c)); };

    connect(chart, &ContrastWidgetImpl::ContrastChart::contrastChanged, this, lamChart2Spin);

}

// These are all passthru to chart
void ContrastWidget::setContrast(ROIVert::contrast c) { chart->setValues(c); }
ROIVert::contrast ContrastWidget::getContrast() { return chart->getValues(); }
void ContrastWidget::setHistogram(QVector<float> y) { chart->setHistogram(y); }
void ContrastWidget::setHistogramColor(QColor c) { chart->setHistogramColor(c); }
void ContrastWidget::setLineColor(QColor c) { chart->setLineColor(c); }
void ContrastWidget::setGammaRange(qreal mingamma, qreal maxgamma) { chart->setGammaRange(mingamma, maxgamma); };