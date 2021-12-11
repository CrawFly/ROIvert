#include <QMainWindow>
#include <QtTest/QtTest>

#include "tTraceChartWidget.h"
#include "widgets/TraceChartWidget.h"
#include "ChartStyle.h"
#include "opencv2/opencv.hpp"

namespace {
    std::shared_ptr<TraceChartSeries> makeSeriesHelper(double xmin, double xmax, std::vector<float> yvalues, std::shared_ptr<ChartStyle> style) {

        cv::Mat data(1, yvalues.size(), CV_32F);
        for (size_t i = 0; i < yvalues.size(); ++i) {
            data.at<float>(0, i) = yvalues[i];
        }
        auto ret= std::make_shared<TraceChartSeries>(style);
        ret->setData(data);
        ret->setXMin(xmin);
        ret->setXMax(xmax);

        return ret;
    }
}
void tTraceChartWidget::init() {
    defstyle = std::make_shared<ChartStyle>();
    chart = new TraceChartWidget(defstyle);
    xaxis = chart->getXAxis();
    xaxis = chart->getXAxis();
    yaxis = chart->getYAxis();
}
void tTraceChartWidget::cleanup() {
    delete chart;
    chart = nullptr;
}

void tTraceChartWidget::tstyle() {
    auto stylea = std::make_shared<ChartStyle>();
    auto styleb = std::make_shared<ChartStyle>();

    chart->setStyle(stylea);
    QCOMPARE(chart->getStyle(), stylea.get());

    chart->setStyle(styleb);
    QCOMPARE(chart->getStyle(), styleb.get());

    auto chartb = std::make_unique<TraceChartWidget>(styleb);
    QCOMPARE(chartb->getStyle(), styleb.get());

    // the style should fan out to x and y axis, to test this use a proxy that
    // the style was set...axis thickness following a change in font size.
    stylea->setTickLabelFontSize(5);
    styleb->setTickLabelFontSize(25);
    chart->setStyle(stylea);
    chart->updateStyle();
    int xthicka = xaxis->getThickness();
    int ythicka = yaxis->getThickness();

    chart->setStyle(styleb);
    chart->updateStyle();
    int xthickb = xaxis->getThickness();
    int ythickb = yaxis->getThickness();
    
    QVERIFY(xthickb > xthicka);
    QVERIFY(ythickb > ythicka);
}

void tTraceChartWidget::tnormalization_data() {
    
    QTest::addColumn<int>("norm");
    QTest::addColumn<QString>("label");
    QTest::addColumn<float>("ymin");
    QTest::addColumn<float>("ymax");

    QTest::newRow("L1NORM") << (int)ROIVert::NORMALIZATION::L1NORM << "df/f (L1 Norm)" << 0.f << 0.4f;
    QTest::newRow("L2NORM") << (int)ROIVert::NORMALIZATION::L2NORM << "df/f (L2 Norm)" << 0.f << .73f;
    QTest::newRow("MEDIQR") << (int)ROIVert::NORMALIZATION::MEDIQR << "df/f (IQR units)" << -1.f << 1.f;
    QTest::newRow("NONE") << (int)ROIVert::NORMALIZATION::NONE << "df/f" << 0.f << 4.f;
    QTest::newRow("ZEROTOONE") << (int)ROIVert::NORMALIZATION::ZEROTOONE << "df/f (0-1)" << 0.f << 1.f;
    QTest::newRow("ZSCORE") << (int)ROIVert::NORMALIZATION::ZSCORE << "df/f (z units)" << -std::sqrtf(2.f) << std::sqrtf(2.f);
}

void tTraceChartWidget::tnormalization() {
    QFETCH(int, norm);
    QFETCH(QString, label);
    QFETCH(float, ymin);
    QFETCH(float, ymax);
    
    auto style = std::make_shared<ChartStyle>();
    auto series = makeSeriesHelper(0, 1, { 0.f, 1.f, 2.f, 3.f, 4.f }, style);
    chart->addSeries(series);

    style->setNormalization(static_cast<ROIVert::NORMALIZATION>(norm));
    chart->setStyle(style);
    chart->updateStyle();

    QCOMPARE(yaxis->getLabel(), label);
    QVERIFY(std::abs(ymin - series->getYMin()) < .001);
    QVERIFY(std::abs(ymax - series->getYMax()) < .001);
}

void tTraceChartWidget::taddremoveseries() {
    auto series1 = makeSeriesHelper(1., 3., { 0.f, 2.f }, defstyle);
    auto series2 = makeSeriesHelper(5., 7., { 3.f, 4.f }, defstyle);

    QCOMPARE(std::get<0>(xaxis->getExtents()), 0);
    QCOMPARE(std::get<1>(xaxis->getExtents()), 1);
    QCOMPARE(std::get<0>(yaxis->getExtents()), 0);
    QCOMPARE(std::get<1>(yaxis->getExtents()), 1);
    QCOMPARE(chart->getSeries().size(), 0);

    chart->addSeries(series1);
    QCOMPARE(std::get<0>(xaxis->getExtents()), 1);
    QCOMPARE(std::get<1>(xaxis->getExtents()), 3);
    QCOMPARE(std::get<0>(yaxis->getExtents()), 0);
    QCOMPARE(std::get<1>(yaxis->getExtents()), 2);
    QCOMPARE(chart->getSeries().size(), 1);
    QCOMPARE(chart->getSeries()[0].get(), series1.get());
    
    chart->addSeries(series2);
    QCOMPARE(std::get<0>(xaxis->getExtents()), 1);
    QCOMPARE(std::get<1>(xaxis->getExtents()), 7);
    QCOMPARE(std::get<0>(yaxis->getExtents()), 0);
    QCOMPARE(std::get<1>(yaxis->getExtents()), 4);
    QCOMPARE(chart->getSeries().size(), 2);
    QCOMPARE(chart->getSeries()[0].get(), series1.get());
    QCOMPARE(chart->getSeries()[1].get(), series2.get());

    chart->removeSeries(series1);
    QCOMPARE(std::get<0>(xaxis->getExtents()), 5);
    QCOMPARE(std::get<1>(xaxis->getExtents()), 7);
    QCOMPARE(std::get<0>(yaxis->getExtents()), 3);
    QCOMPARE(std::get<1>(yaxis->getExtents()), 4);
    QCOMPARE(chart->getSeries().size(), 1);
    QCOMPARE(chart->getSeries()[0].get(), series2.get());
}

void tTraceChartWidget::ttitle() {
    QCOMPARE(chart->getTitle(), "");
    chart->setTitle("ABCD");
    QCOMPARE(chart->getTitle(), "ABCD");
}

void tTraceChartWidget::tantialiasing() {
    chart->setAntiAliasing(true);
    QCOMPARE(chart->getAntiAliasing(), true);
    chart->setAntiAliasing(false);
    QCOMPARE(chart->getAntiAliasing(), false);
}

void tTraceChartWidget::tclick() {
    // this test is going to depend on painting, so:
    //  set up a main window
    //  use a separate chart object as deleting the window will destroy the chart
    
    auto win = std::make_unique<QMainWindow>();
    win->setFixedSize(500, 500);
    auto testchart = new TraceChartWidget(defstyle, win.get());
    win->setCentralWidget(testchart);

    bool clickfired = false;
    std::vector<TraceChartSeries*> hitseries;
    connect(testchart, &TraceChartWidget::chartClicked,
        [&](TraceChartWidget*, std::vector<TraceChartSeries*> ser, Qt::KeyboardModifiers)
        {
        clickfired = true;
        hitseries = ser;
        }
    );

    auto series1 = makeSeriesHelper(0., 1., { 1.f, 1.f, 0.f, 0.f, 0.f }, defstyle);
    auto series2 = makeSeriesHelper(0., 1., { 0.f, 0.f, 0.f, 1.f, 1.f }, defstyle);
    testchart->addSeries(series1);
    testchart->addSeries(series2);

    
    win->show();    
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    
    //click outside:
    auto rect = testchart->contentsRect();
    QPoint clicklocation(-1, -1);
    QTest::mouseClick(testchart, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, clicklocation);
    QCOMPARE(clickfired, true);
    QCOMPARE(hitseries.size(), 0);

    // click left:
    clickfired = false;
    clicklocation.setX(rect.x() + rect.width() * .25);
    clicklocation.setY(rect.y() + rect.height() * .5);
    QTest::mouseClick(testchart, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, clicklocation);
    QCOMPARE(clickfired, true);
    QCOMPARE(hitseries.size(), 1);
    QCOMPARE(hitseries[0], series1.get());
    
    // click right:
    clickfired = false;
    clicklocation.setX(rect.x() + rect.width() * .75);
    clicklocation.setY(rect.y() + rect.height() * .5);
    QTest::mouseClick(testchart, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, clicklocation);
    QCOMPARE(clickfired, true);
    QCOMPARE(hitseries.size(), 1);
    QCOMPARE(hitseries[0], series2.get());
    
    // click right and get two series:
    auto series3 = makeSeriesHelper(0., 1., { 0.f, 0.f, 0.f, 1.f, 1.f }, defstyle);
    testchart->addSeries(series3);
    clickfired = false;
    clicklocation.setX(rect.x() + rect.width() * .75);
    clicklocation.setY(rect.y() + rect.height() * .5);
    QTest::mouseClick(testchart, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, clicklocation);
    QCOMPARE(clickfired, true);
    QCOMPARE(hitseries.size(), 2);
    QCOMPARE(hitseries[0], series2.get());
    QCOMPARE(hitseries[1], series3.get());
    
    // click middle and get an empty:
    clickfired = false;
    clicklocation.setX(rect.x() + rect.width() * .55);
    clicklocation.setY(rect.y() + rect.height() * .5);
    QTest::mouseClick(testchart, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, clicklocation);
    QCOMPARE(clickfired, true);
    QCOMPARE(hitseries.size(), 0);
}

// todo: saveasimage...feels like it might belong in fileio, needs a file fixture?
// todo: minimumsizehint...waiting for this until more progress on sizing and AR
// todo: series, axis, ridgeline

