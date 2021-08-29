#include "widgets/ContrastWidget.h"
#include <QGraphicsScene>
#include <QPainterPath>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QBoxLayout>
#include "widgets/ContrastWidgetImpl.h"
#include <QDoubleSpinBox>

#include <QDebug>

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
        spinMin.setSingleStep(.05);
        spinMax.setValue(0.);
        spinMin.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinMin.setToolTip(tr("Minimum: pixels less than this proportion of the pixel range will be shown as black."));

        spinMax.setSingleStep(.05);
        spinMax.setValue(1.);
        spinMax.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinMax.setToolTip(tr("Maximum value: pixels greater than this proportion of the pixel range will be shown as white."));

        spinGamma.setValue(1.);
        spinGamma.setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
        spinGamma.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinGamma.setToolTip(tr("Gamma correction: power-law nonlinearity applied to pixel brightness."));
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
    void doConnect(ContrastWidget *par)
    {
        const auto lamSpin2Chart = [=]()
        {
            ROIVert::contrast c{spinMin.value(), spinMax.value(), spinGamma.value()};
            chart.setValues(c);
            emit par->contrastChanged(c);
        };

        const auto lamChart2Spin = [=](ROIVert::contrast c)
        {
            spinMin.setValue(std::get<0>(c));
            spinMax.setValue(std::get<1>(c));
            spinGamma.setValue(std::get<2>(c));
        };

        connect(&spinMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), lamSpin2Chart);
        connect(&spinMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), lamSpin2Chart);
        connect(&spinGamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), lamSpin2Chart);
        connect(&chart, &ContrastWidgetImpl::ContrastChart::contrastChanged, lamChart2Spin);
    }
};

ContrastWidget::ContrastWidget(QWidget *parent) : QWidget(parent)
{
    setGammaRange(.001, 10.);
    impl->init();
    impl->layout();
    impl->doConnect(this);
    setLayout(&impl->lay);
}

ROIVert::contrast ContrastWidget::getContrast() { return impl->chart.getValues(); }
void ContrastWidget::setContrast(ROIVert::contrast c) { impl->chart.setValues(c); }
void ContrastWidget::setHistogram(QVector<float> y) { impl->chart.setHistogram(y); }
void ContrastWidget::setHistogramColor(QColor c) { impl->chart.setHistogramColor(c); }
void ContrastWidget::setLineColor(QColor c) { impl->chart.setLineColor(c); }
void ContrastWidget::setGammaRange(qreal mingamma, qreal maxgamma) { impl->chart.setGammaRange(mingamma, maxgamma); };