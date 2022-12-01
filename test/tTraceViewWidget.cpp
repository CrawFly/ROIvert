#include "tTraceViewWidget.h"

#include <QtTest/QtTest>
#include "dockwidgets/TraceViewWidget.h"
#include "widgets/ChartControlWidget.h"
#include "testUtils.h"
#include "widgets/TraceChartWidget.h"
#include "ChartStyle.h"
#include <QDoubleSpinBox>
#include <QScrollBar>

void tTraceViewWidget::init() {
    tview = new TraceViewWidget;
    chartcontrols = tview->findChild<ChartControlWidget*>("chartcontrols");    
    QVERIFY(chartcontrols);
}
void tTraceViewWidget::cleanup() {
    delete tview;
    tview = nullptr;
}
void tTraceViewWidget::tChartControlsTimeRange()
{
    RidgeLineWidget& ridge{ tview->getRidgeChart() };
    ChartStyle* ridgestyle{ ridge.getStyle() };
    auto cs = std::make_shared<ChartStyle>();
    auto linechart = TraceChartWidget(cs);
            
    auto spinTMin = chartcontrols->findChild<QDoubleSpinBox*>("spinTMin");
    auto spinTMax = chartcontrols->findChild<QDoubleSpinBox*>("spinTMax");
    QVERIFY(spinTMin);
    QVERIFY(spinTMax);
    
    tview->updateTMax(3.14f);
    QCOMPARE(spinTMax->value(), 3.14);
    QCOMPARE(spinTMin->maximum(), 3.13);
    QCOMPARE(chartcontrols->getTMin(), 0);
    QCOMPARE(chartcontrols->getTMax(), 3.14);

    tview->addLineChart(&linechart); // note: add the line chart AFTER setting a TMax with new charts to test auto limits to span of video

    spinTMax->setValue(2.9);
    spinTMin->setValue(2.2);
    QCOMPARE(spinTMax->minimum(), 2.21);
    QCOMPARE(spinTMin->maximum(), 2.89);
    QCOMPARE(std::get<0>(ridge.getXAxis()->getLimits()), 2.2);
    QCOMPARE(std::get<1>(ridge.getXAxis()->getLimits()), 2.9);
    QCOMPARE(ridgestyle->getXLimitStyle(), ROIVert::LIMITSTYLE::MANAGED);
    QCOMPARE(linechart.getStyle()->getXLimitStyle(), ROIVert::LIMITSTYLE::MANAGED);
    QCOMPARE(chartcontrols->getTMin(), 2.2);
    QCOMPARE(chartcontrols->getTMax(), 2.9);

    auto themax = tview->makeAllTimeLimitsAuto();
    nearlyequal(themax, 3.14);
    QCOMPARE(std::get<0>(ridge.getXAxis()->getLimits()), 0.);
    QCOMPARE(std::get<1>(ridge.getXAxis()->getLimits()), 1.);
    QCOMPARE(ridgestyle->getXLimitStyle(), ROIVert::LIMITSTYLE::AUTO);
    QCOMPARE(linechart.getStyle()->getXLimitStyle(), ROIVert::LIMITSTYLE::AUTO);


    chartcontrols->setTRange(2.4, 2.5);
    QCOMPARE(spinTMin->value(), 2.4);
    QCOMPARE(spinTMax->value(), 2.5);
    QCOMPARE(chartcontrols->getTMin(), 2.4);
    QCOMPARE(chartcontrols->getTMax(), 2.5);
}

void tTraceViewWidget::tChartControlsChartHeight()
{

    auto cs = std::make_shared<ChartStyle>();

    auto chart1 = TraceChartWidget(cs);
    auto chart2 = TraceChartWidget(cs);
    auto chart3 = TraceChartWidget(cs);
    
    tview->addLineChart(&chart1);
    tview->addLineChart(&chart2);
    tview->addLineChart(&chart3);
    auto initheight = chart1.height();
    QCOMPARE(chart2.height(), initheight);
    QCOMPARE(chart3.height(), initheight);
    
    QVERIFY(initheight != 314);

    chartcontrols->changeLineChartHeight(314);
    QCOMPARE(chart1.height(), 314);
    QCOMPARE(chart2.height(), 314);
    QCOMPARE(chart3.height(), 314);

    auto chart4 = TraceChartWidget(cs);
    tview->addLineChart(&chart4);
    QCOMPARE(chart4.height(), 314);
}


void tTraceViewWidget::tScrollToChart() {
    tview->show();
    std::vector<std::unique_ptr<TraceChartWidget>> chartstack;
    auto cs = std::make_shared<ChartStyle>();
    for (size_t i = 0; i < 15; ++i) {
        chartstack.emplace_back(std::make_unique<TraceChartWidget>(cs, tview));
        tview->addLineChart(chartstack.back().get());
    }

    auto scrollarea = tview->findChild<RScrollArea*>();
    tview->scrollToChart(chartstack[0].get());
    auto pos1 = scrollarea->verticalScrollBar()->value();
    tview->scrollToChart(chartstack[14].get());
    auto pos2 = scrollarea->verticalScrollBar()->value();
    QCOMPARE(pos1, 0);
    QVERIFY(pos2 > pos1);
}
