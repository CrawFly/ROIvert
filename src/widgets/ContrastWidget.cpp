#include "widgets/ContrastWidget.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>

#include "widgets/ContrastWidgetImpl.h"


struct ContrastWidget::pimpl
{
    ContrastWidgetImpl::ContrastChart chart;
    QDoubleSpinBox spinMin;
    QDoubleSpinBox spinGamma;
    QDoubleSpinBox spinMax;

    QVBoxLayout lay;

    void init()
    {
        lay.setContentsMargins(QMargins(0, 0, 0, 0));
        chart.setToolTip(tr("A histogram of image data is shown. Slide the minimum/maximum/gamma lines to adjust contrast or set values in the boxes below."));
        spinMin.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinMin.setToolTip(tr("Minimum: pixels less than this proportion of the pixel range will be shown as black."));
        spinMin.setSingleStep(.05);
        spinMin.setValue(0.);
        spinMin.setObjectName("spinMin");

        spinMax.setSingleStep(.05);
        spinMax.setValue(1.);
        spinMax.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinMax.setToolTip(tr("Maximum value: pixels greater than this proportion of the pixel range will be shown as white."));
        spinMax.setObjectName("spinMax");
        
        spinGamma.setValue(1.);
        spinGamma.setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
        spinGamma.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinGamma.setToolTip(tr("Gamma correction: power-law nonlinearity applied to pixel brightness."));
        spinGamma.setObjectName("spinGamma");
    }
    void layout()
    {
        lay.addWidget(&chart);

        {
            auto layEdit = std::make_unique<QHBoxLayout>();
            layEdit->addWidget(&spinMin);
            layEdit->addStretch(1);
            layEdit->addWidget(&spinGamma);
            layEdit->addStretch(1);
            layEdit->addWidget(&spinMax);
            lay.addLayout(layEdit.release());
        }
    }


};

ContrastWidget::ContrastWidget(QWidget *parent) : QWidget(parent), impl(std::make_unique<pimpl>())
{
    setGammaRange(.001, 10.);
    impl->init();
    impl->layout();
    setLayout(&impl->lay);

    connect(&impl->spinMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ContrastWidget::spin2Chart);
    connect(&impl->spinMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,  &ContrastWidget::spin2Chart);
    connect(&impl->spinGamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ContrastWidget::spin2Chart);
    connect(&impl->chart, &ContrastWidgetImpl::ContrastChart::contrastChanged, this, &ContrastWidget::chart2Spin);
}

ContrastWidget::~ContrastWidget() { }


void ContrastWidget::spin2Chart() {
    ROIVert::contrast c{impl->spinMin.value(), impl->spinMax.value(), impl->spinGamma.value()};
    impl->chart.setValues(c, true);
    emit contrastChanged(c);
}
void ContrastWidget::chart2Spin(ROIVert::contrast c) {
    
    impl->spinMin.blockSignals(true);
    impl->spinMax.blockSignals(true);
    impl->spinGamma.blockSignals(true);

    impl->spinMin.setValue(std::get<0>(c));
    impl->spinMax.setValue(std::get<1>(c));
    impl->spinGamma.setValue(std::get<2>(c));

    impl->spinMin.blockSignals(false);
    impl->spinMax.blockSignals(false);
    impl->spinGamma.blockSignals(false);

    emit contrastChanged(c);

}


ROIVert::contrast ContrastWidget::getContrast() { 
    return impl->chart.getValues(); 
}

void ContrastWidget::setContrast(ROIVert::contrast c) { 
    impl->chart.setValues(c, true);
    chart2Spin(c);
}



void ContrastWidget::setHistogram(QVector<float> y) { impl->chart.setHistogram(y); }
void ContrastWidget::setHistogramColor(QColor c) { impl->chart.setHistogramColor(c); }
void ContrastWidget::setLineColor(QColor c) { impl->chart.setLineColor(c); }
void ContrastWidget::setGammaRange(qreal mingamma, qreal maxgamma) { impl->chart.setGammaRange(mingamma, maxgamma); };
