// todo: when the chart style changes chartcontrolwidget neeeds to re-evaluate its minimum height

#include "widgets/ChartControlWidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include <QPushButton>
#include "dockwidgets/TraceViewWidget.h"

struct ChartControlWidget::pimpl {
    QHBoxLayout toplay;
    QSpinBox spinHeight;
    QDoubleSpinBox spinTMin;
    QDoubleSpinBox spinTMax;
    QPushButton cmdAutoTimeRange;
    TraceViewWidget* tview;

    void configureWidgets() {
        spinHeight.setFixedWidth(100);
        spinHeight.setMaximum(999999);
        spinHeight.setMinimum(50);
        spinHeight.setToolTip(tr("Height of the line charts in pixels."));
        cmdAutoTimeRange.setText(tr("Auto"));
        cmdAutoTimeRange.setCheckable(true);
        cmdAutoTimeRange.setChecked(true);

    }

    void doLayout() {
        toplay.addWidget(new QLabel(tr("Line Chart Height:")));
        toplay.addWidget(&spinHeight);
        toplay.addStretch(1);
        toplay.addWidget(new QLabel(tr("Time Range:")));
        toplay.addWidget(&spinTMin);
        toplay.addWidget(new QLabel("-"));
        toplay.addWidget(&spinTMax);
        toplay.addWidget(&cmdAutoTimeRange);
    }

    void adjustTimeSpinners(ChartControlWidget* w) {
        emit w->timeRangeChanged(spinTMin.value(), spinTMax.value());
        spinTMin.setMaximum(spinTMax.value());
        spinTMax.setMinimum(spinTMin.value() + .01);
        cmdAutoTimeRange.setChecked(false); 
    }
};

ChartControlWidget::ChartControlWidget(TraceViewWidget* par) : impl(std::make_unique<pimpl>()) {
    impl->tview = par;
    setLayout(&impl->toplay);
    impl->doLayout();
    impl->configureWidgets();

    connect(&impl->spinHeight, QOverload<int>::of(&QSpinBox::valueChanged), [&](int val) { emit lineChartHeightChanged(val); });


    connect(&impl->spinTMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val) { impl->adjustTimeSpinners(this);  });
    connect(&impl->spinTMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val) { impl->adjustTimeSpinners(this);  });
    connect(&impl->cmdAutoTimeRange, &QPushButton::clicked, this, &ChartControlWidget::setAutoTMax);
}

ChartControlWidget::~ChartControlWidget() {
}

void ChartControlWidget::changeMinimumLineChartHeight(int minheight) {
    impl->spinHeight.setMinimum(minheight);
}

void ChartControlWidget::changeLineChartHeight(int newheight) {
    impl->spinHeight.setValue(newheight);
}

void ChartControlWidget::setAutoTMax() {
    if (impl->cmdAutoTimeRange.isChecked()) {
        impl->spinTMin.blockSignals(true);
        impl->spinTMax.blockSignals(true);
        auto tmax = impl->tview->makeAllTimeLimitsAuto();
        impl->spinTMin.setValue(0);
        impl->spinTMin.setMaximum(tmax);
        impl->spinTMax.setValue(tmax);
        impl->spinTMax.setMinimum(0);
        impl->spinTMin.blockSignals(false);
        impl->spinTMax.blockSignals(false);
    }
}
double ChartControlWidget::getTMin() const {
    return impl->spinTMin.value();
}
double ChartControlWidget::getTMax() const {
    return impl->spinTMax.value();
}