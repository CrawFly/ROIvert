#include "dockwidgets/TraceViewWidget.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QKeyEvent>
#include <QSize>
#include <QTabWidget>

#include "ChartStyle.h"
#include "ROIVertEnums.h"
#include "widgets/ChartControlWidget.h"
#include "widgets/TraceChartWidget.h"

void RScrollArea::wheelEvent(QWheelEvent* event) {
    if (event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
        emit modwheel(event->angleDelta().y());
        return;
    }
    QScrollArea::wheelEvent(event);
}

struct TraceViewWidget::pimpl
{
    std::shared_ptr<ChartStyle> coreLineStyle = std::make_shared<ChartStyle>();
    std::shared_ptr<ChartStyle> coreRidgeStyle = std::make_shared<ChartStyle>();
    std::unique_ptr<QGridLayout> topGridLayout = std::make_unique<QGridLayout>();
    std::unique_ptr<QVBoxLayout> lineChartLayout = std::make_unique<QVBoxLayout>();
    std::unique_ptr<RidgeLineWidget> ridgeChart = std::make_unique<RidgeLineWidget>();
    std::unique_ptr<ChartControlWidget> chartcontrols;
    RScrollArea* scrollArea{ new RScrollArea };
    int linechartheight = 0;

    void init(TraceViewWidget* par) {
        chartcontrols = std::make_unique<ChartControlWidget>(par);
    }
    void doLayout()
    {
        topGridLayout->addWidget(chartcontrols.get());
        topGridLayout->addWidget(tab);
        topGridLayout->setContentsMargins(10,0,10,10);

        tab->addTab(tabLine, "Line");
        tab->addTab(tabRidgeLine, "Ridge");
        //tab->addTab(tabImage, "Image");

        tabLine->setLayout(scrollAreaParent);
        scrollAreaParent->addWidget(scrollArea);
        scrollAreaContent->setLayout(lineChartLayout.get());
        scrollArea->setWidget(scrollAreaContent);

        scrollArea->setWidgetResizable(true);
        scrollAreaParent->setContentsMargins(0, 0, 0, 0);
        lineChartLayout->setContentsMargins(0, 0, 0, 0);
        lineChartLayout->setSpacing(10);
        lineChartLayout->setMargin(10);
        lineChartLayout->setAlignment(Qt::AlignTop);

        tabRidgeLine->setLayout(ridgeLayout);
        ridgeLayout->addWidget(ridgeChart.get());
    }

    void scrollToWidget(QWidget* w)
    {
        scrollArea->ensureWidgetVisible(w);
    }

    QList<TraceChartWidget*> getLineCharts() {
        QList<TraceChartWidget*> ret;
        auto n = lineChartLayout->count();
        ret.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            auto chart = dynamic_cast<TraceChartWidget*>(lineChartLayout->itemAt(i)->widget());
            if (chart) {
                ret.push_back(chart);
            }
        }
        return ret;
    }

    void wheelToScroll(int delta) {
        if (!getLineCharts().empty()) {
            auto newheight = linechartheight + delta / 2;
            chartcontrols->changeLineChartHeight(newheight);
        }
    }

    void setChartHeight(int newheight) {
        auto charts = getLineCharts();
        if (!charts.empty()) {
            auto minheight = charts[0]->minimumSizeHint().height();
            if (newheight < minheight) {
                chartcontrols->changeMinimumLineChartHeight(minheight);
                newheight = minheight;
            }
            linechartheight = newheight;
            for (auto& chart : charts) {
                chart->setFixedHeight(linechartheight);
            }
        }
    }

    void setTimeRange(double tmin, double tmax) {
        auto charts = getLineCharts();
        for (auto& chart : charts) {
            auto cs = chart->getStyle();
            cs->setXLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
            chart->getXAxis()->setManualLimits(tmin, tmax);
            auto ser = chart->getSeries()[0];
            chart->updateStyle();
        }
        coreLineStyle->setXLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
        coreRidgeStyle->setXLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
        ridgeChart->getXAxis()->setManualLimits(tmin, tmax);
        ridgeChart->updateStyle();
    };

    float tmaxifnocharts;
private:
    // todo: make these all unique/scoped (be careful with order)
    QTabWidget* tab{ new QTabWidget };
    QWidget* tabLine{ new QWidget };
    QWidget* tabRidgeLine{ new QWidget };
    QWidget* tabImage{ new QWidget };
    QGridLayout* scrollAreaParent{ new QGridLayout };
    QWidget* scrollAreaContent{ new QWidget };
    QGridLayout* ridgeLayout{ new QGridLayout };
};

TraceViewWidget::TraceViewWidget(QWidget* parent) :
    QDockWidget(parent),
    impl(std::make_unique<pimpl>())
{
    impl->init(this);
    auto contents = new QWidget;
    this->setWidget(contents);
    contents->setLayout(impl->topGridLayout.get());

    impl->coreRidgeStyle->setDoBackBrush(true);
    impl->coreRidgeStyle->setNormalization(ROIVert::NORMALIZATION::ZEROTOONE);
    impl->coreRidgeStyle->setYLimitStyle(ROIVert::LIMITSTYLE::TIGHT);
    impl->ridgeChart->setStyle(impl->coreRidgeStyle);

    impl->doLayout();

    connect(impl->scrollArea, &RScrollArea::modwheel, [&](int delta) { impl->wheelToScroll(delta); });
    connect(impl->chartcontrols.get(), &ChartControlWidget::lineChartHeightChanged, [&](int newheight) { impl->setChartHeight(newheight); });
    connect(impl->chartcontrols.get(), &ChartControlWidget::timeRangeChanged, [&](double tmin, double tmax) { impl->setTimeRange(tmin, tmax); });
    connect(impl->ridgeChart.get(), &TraceChartWidget::chartTimeLimitsSelected, [&](double min, double max) { impl->chartcontrols->setTRange(min, max); });
}

TraceViewWidget::~TraceViewWidget() = default;

void TraceViewWidget::addLineChart(TraceChartWidget * chart)
{
    impl->lineChartLayout->addWidget(chart);
    chart->getXAxis()->setLabel("Time (s)");

    // This aggressive double update is required to ensure that the scroll area
    // is up to date before scrolling to the new chart.
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    if (chart->getStyle()->getXLimitStyle() == ROIVert::LIMITSTYLE::MANAGED) {
        chart->getXAxis()->setManualLimits(impl->chartcontrols->getTMin(), impl->chartcontrols->getTMax());
    }

    impl->setChartHeight(impl->linechartheight);
    impl->scrollToWidget(chart);

    connect(chart, &TraceChartWidget::chartTimeLimitsSelected, [&](double min, double max) { impl->chartcontrols->setTRange(min, max); });
}
void TraceViewWidget::scrollToChart(TraceChartWidget * w)
{
    impl->scrollToWidget(w);
}
RidgeLineWidget& TraceViewWidget::getRidgeChart() noexcept
{
    return *(impl->ridgeChart);
}
ChartStyle* TraceViewWidget::getCoreRidgeChartStyle() const noexcept
{
    return impl->coreRidgeStyle.get();
}
ChartStyle* TraceViewWidget::getCoreLineChartStyle() const noexcept
{
    return impl->coreLineStyle.get();
}
void TraceViewWidget::keyPressEvent(QKeyEvent * event)
{
    emit keyPressed(event->key(), event->modifiers());
}
void TraceViewWidget::mousePressEvent(QMouseEvent * event)
{
    emit chartClicked(nullptr, std::vector<TraceChartSeries*>(), event->modifiers());
}
void TraceViewWidget::updateMinimumHeight() {
    auto charts = impl->getLineCharts();
    if (!charts.empty()) {
        auto minheight = charts[0]->minimumSizeHint().height();
        impl->chartcontrols->changeMinimumLineChartHeight(minheight);
    }
}
void TraceViewWidget::updateTMax(float tmax) {
    impl->tmaxifnocharts = tmax;
    impl->chartcontrols->setAutoTMax();

}
double TraceViewWidget::makeAllTimeLimitsAuto() {
    auto charts = impl->getLineCharts();
    if (charts.empty()) {
        return impl->tmaxifnocharts;
    }
    for (auto& chart : charts) {
        auto cs = chart->getStyle();
        cs->setXLimitStyle(ROIVert::LIMITSTYLE::AUTO);
        chart->updateStyle();
    }
    impl->coreLineStyle->setXLimitStyle(ROIVert::LIMITSTYLE::AUTO);
    impl->coreRidgeStyle->setXLimitStyle(ROIVert::LIMITSTYLE::AUTO);
    impl->ridgeChart->updateStyle();
    return std::get<1>(impl->ridgeChart->getXAxis()->getLimits());
}