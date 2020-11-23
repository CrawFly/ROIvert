#include "contrastwidget.h"
#include "qdebug.h"



ContrastWidget::ContrastWidget(QWidget* parent) {
        setParent(parent);
        QVBoxLayout* layV = new QVBoxLayout;
        contChart = new ContrastHistogramChart;
        layV->addWidget(contChart);

        QHBoxLayout* layH = new QHBoxLayout;
        spinMin->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        spinGamma->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        spinMax->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        layH->addWidget(spinMin, 0, Qt::AlignLeft);
        layH->addWidget(spinGamma, 0, Qt::AlignHCenter);
        layH->addWidget(spinMax, 0, Qt::AlignRight);

        spinMin->setMinimum(0.);
        spinMin->setMaximum(1.);
        spinMin->setValue(0.);
        spinMin->setSingleStep(.01);

        spinGamma->setMinimum(0.001);
        spinGamma->setMaximum(10.);
        spinGamma->setValue(1.);
        spinGamma->setSingleStep(.01);
        contChart->setGammaRange(.001, 10.);

        spinMax->setMinimum(0.);
        spinMax->setMaximum(1.);
        spinMax->setValue(1.);
        spinMax->setSingleStep(.01);

        layV->addLayout(layH);
        setLayout(layV);
        layV->setAlignment(Qt::AlignTop);

        connect(contChart, &ContrastHistogramChart::minChanged, this, [=](double val) {spinMin->setValue(val); });
        connect(contChart, &ContrastHistogramChart::maxChanged, this, [=](double val) {spinMax->setValue(val); });
        connect(contChart, &ContrastHistogramChart::gammaChanged, this, [=](double val) {spinGamma->setValue(val); });

        connect(spinMin, SIGNAL(valueChanged(double)), contChart, SLOT(changeMin(double)));
        connect(spinMax, SIGNAL(valueChanged(double)), contChart, SLOT(changeMax(double)));
        connect(spinGamma, SIGNAL(valueChanged(double)), contChart, SLOT(changeGamma(double)));

        connect(spinMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { emit contrastChanged(getMin(), getMax(), getGamma());});
        connect(spinMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { emit contrastChanged(getMin(), getMax(), getGamma());});
        connect(spinGamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { emit contrastChanged(getMin(), getMax(), getGamma());});

    }

ContrastWidget::~ContrastWidget() {};

double ContrastWidget::getMin() { return spinMin->value(); }
double ContrastWidget::getMax() { return spinMax->value(); }
double ContrastWidget::getGamma() { return spinGamma->value(); }
void ContrastWidget::setHist(std::vector<float> histval) {contChart->setData(histval);}

