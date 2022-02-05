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
        auto ret = std::make_shared<TraceChartSeries>(style);
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

    style->setNormalization(static_cast<ROIVert::NORMALIZATION>(norm + 1 % 6));
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

void tTraceChartWidget::tseriesdata() {
    // series extents tested elsewhere, test set/get with a swap:
    auto series1 = makeSeriesHelper(0, 1, { 0.f, 1.f }, defstyle);
    auto series2 = makeSeriesHelper(2, 3, { 2.f, 3.f }, defstyle);

    cv::Mat data1 = series1->getData();
    cv::Mat data2 = series2->getData();

    series1->setData(data2);
    series2->setData(data1);

    QCOMPARE(series1->getData().at<float>(0), 2.f);
    QCOMPARE(series1->getData().at<float>(1), 3.f);
    QCOMPARE(series2->getData().at<float>(0), 0.f);
    QCOMPARE(series2->getData().at<float>(1), 1.f);

    // coercion out of float test:
    cv::Mat dataint(1, 2, CV_8U);
    dataint.at<uint8_t>(0, 0) = 1;
    dataint.at<uint8_t>(0, 1) = 2;
    series1->setData(dataint);
    QCOMPARE(series1->getData().at<float>(0), 1.f);
    QCOMPARE(series1->getData().at<float>(1), 2.f);
}
void tTraceChartWidget::tseriesextents() {
    double xmin = 2., xmax = 3.;
    float ymin = 4., ymax = 5.;
    auto series = makeSeriesHelper(xmin, xmax, { ymin, ymax, (ymin + ymax) / 2.f }, defstyle);

    QCOMPARE(series->getXMin(), xmin);
    QCOMPARE(series->getXMax(), xmax);
    QCOMPARE(series->getYMin(), ymin);
    QCOMPARE(series->getYMax(), ymax);

    QCOMPARE(series->getExtents(), QRectF(QPointF(xmin, ymin), QPointF(xmax, ymax)));
}

void tTraceChartWidget::tseriesoffset() {
    auto series = makeSeriesHelper(0, 1, { 1.f, 2.f }, defstyle);
    series->setOffset(4.2f);
    QCOMPARE(series->getOffset(), 4.2f);
    QCOMPARE(series->getYMin(), 5.2f);
    QCOMPARE(series->getYMax(), 6.2f);
}

void tTraceChartWidget::tseriesdegendata() {
    // absent data for a series:
    TraceChartSeries nodataseries;
    QCOMPARE(nodataseries.getYMin(), 0);
    QCOMPARE(nodataseries.getYMax(), 1);

    auto style = std::make_shared<ChartStyle>();
    auto novarseries = makeSeriesHelper(0., 1., { 1.5f, 1.5f, 1.5f, 1.5f }, style);
    QCOMPARE(novarseries->getYMin(), 0.5);
    QCOMPARE(novarseries->getYMax(), 2.5);

    style->setNormalization(ROIVert::NORMALIZATION::MEDIQR);
    novarseries->updatePoly();
    QCOMPARE(novarseries->getYMin(), -1.);
    QCOMPARE(novarseries->getYMax(), 1.);

    style->setNormalization(ROIVert::NORMALIZATION::ZSCORE);
    novarseries->updatePoly();
    QCOMPARE(novarseries->getYMin(), -1.);
    QCOMPARE(novarseries->getYMax(), 1.);
}

void tTraceChartWidget::tseriessetstyle() {
    auto style1 = std::make_shared<ChartStyle>();
    auto style2 = std::make_shared<ChartStyle>();
    style1->setNormalization(ROIVert::NORMALIZATION::NONE);
    style2->setNormalization(ROIVert::NORMALIZATION::ZEROTOONE);

    auto series = makeSeriesHelper(0, 1, { 1., 100. }, style1);
    QCOMPARE(series->getYMax(), 100.);

    series->setStyle(style2);
    series->updatePoly();
    QVERIFY(std::abs(series->getYMax() - 1.) < .001);
}

void tTraceChartWidget::taxislimits() {
    auto style = std::make_shared<ChartStyle>();
    style->setXLimitStyle(ROIVert::LIMITSTYLE::AUTO);
    TraceChartVAxis ax(style);
    ax.setExtents(1., 2.);
    QCOMPARE(std::get<0>(ax.getExtents()), 1.);
    QCOMPARE(std::get<1>(ax.getExtents()), 2.);
    QCOMPARE(std::get<0>(ax.getLimits()), 1.);
    QCOMPARE(std::get<1>(ax.getLimits()), 2.);

    ax.setExtents(-std::sqrt(2), 3.49);
    QCOMPARE(std::get<0>(ax.getExtents()), -std::sqrt(2));
    QCOMPARE(std::get<1>(ax.getExtents()), 3.49);
    QCOMPARE(std::get<0>(ax.getLimits()), -1.5);
    QCOMPARE(std::get<1>(ax.getLimits()), 3.5);

    style->setYLimitStyle(ROIVert::LIMITSTYLE::TIGHT);
    QCOMPARE(std::get<0>(ax.getLimits()), -std::sqrt(2));
    QCOMPARE(std::get<1>(ax.getLimits()), 3.49);

    style->setYLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
    ax.setManualLimits(4., 5.);
    QCOMPARE(std::get<0>(ax.getLimits()), 4.);
    QCOMPARE(std::get<1>(ax.getLimits()), 5.);

    style->setNormalization(ROIVert::NORMALIZATION::ZEROTOONE);
    QCOMPARE(std::get<0>(ax.getLimits()), 0.);
    QCOMPARE(std::get<1>(ax.getLimits()), 1.);

    // todo: test for hax non-auto limits
    TraceChartHAxis hax(style);
    hax.setExtents(-std::sqrt(2), 3.49);
    QCOMPARE(std::get<0>(hax.getExtents()), -std::sqrt(2));
    QCOMPARE(std::get<1>(hax.getExtents()), 3.49);
    QCOMPARE(std::get<0>(hax.getLimits()), -1.5);
    QCOMPARE(std::get<1>(hax.getLimits()), 3.5);
}

void tTraceChartWidget::taxisticks() {
    auto style = std::make_shared<ChartStyle>();
    TraceChartVAxis ax(style);
    ax.setExtents(0, 10.);

    // the actual ticks will be up to 2 more than the setting...
    ax.setMaxNTicks(5);
    QCOMPARE(ax.getTickValues(), std::vector<double>({ 0., 2., 4., 6., 8., 10. }));
    ax.setMaxNTicks(11);
    QCOMPARE(ax.getTickValues(), std::vector<double>({ 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10. }));
    ax.setMaxNTicks(20);
    QCOMPARE(ax.getTickValues(), std::vector<double>({ 0., .5, 1., 1.5, 2., 2.5, 3., 3.5, 4., 4.5, 5., 5.5, 6., 6.5, 7., 7.5, 8., 8.5, 9., 9.5, 10. }));
    int withdec = ax.getThickness();

    ax.setMaxNTicks(2);
    QCOMPARE(ax.getTickValues(), std::vector<double>({ 0., 5., 10. }));
    int withoutdec = ax.getThickness();

    QVERIFY(withoutdec < withdec);
}

void tTraceChartWidget::taxisthickness_data() {
    QTest::addColumn<QString>("font");
    QTest::addColumn<int>("tickfontsize");
    QTest::addColumn<int>("lblfontsize");
    QTest::addColumn<int>("ticklength");
    QTest::addColumn<int>("labelspacing");
    QTest::addColumn<int>("ticklabelspacing");
    QTest::addColumn<int>("tickmarkspacing");
    QTest::addColumn<QString>("label");
    QTest::newRow("1") << "Arial" << 5 << 10 << 3 << 1 << 2 << 3 << "ABC";
    QTest::newRow("2") << "Arial" << 10 << 10 << 3 << 1 << 2 << 3 << "ABC";
    QTest::newRow("3") << "Arial" << 5 << 20 << 3 << 1 << 2 << 3 << "ABC";
    QTest::newRow("4") << "Arial" << 10 << 20 << 3 << 1 << 2 << 3 << "ABC";
    QTest::newRow("5") << "Arial" << 5 << 10 << 5 << 7 << 3 << 5 << "ABC";
    QTest::newRow("6") << "Arial" << 10 << 10 << 6 << 9 << 4 << 6 << "ABC";
    QTest::newRow("7") << "Arial" << 5 << 20 << 7 << 9 << 5 << 7 << "ABC";
    QTest::newRow("8") << "Arial" << 10 << 20 << 9 << 10 << 6 << 8 << "ABC";
    QTest::newRow("9") << "Arial" << 5 << 20 << 7 << 9 << 5 << 7 << "ABC";
    QTest::newRow("10") << "Arial" << 5 << 20 << 7 << 9 << 5 << 7 << "ABCDEFG";
    QTest::newRow("11") << "Courier" << 5 << 20 << 7 << 9 << 5 << 7 << "ABC";
}

void tTraceChartWidget::taxisthickness() {
    // return + ticklength;
    QFETCH(QString, font);
    QFETCH(int, tickfontsize);
    QFETCH(int, lblfontsize);
    QFETCH(int, ticklength);
    QFETCH(int, labelspacing);
    QFETCH(int, ticklabelspacing);
    QFETCH(int, tickmarkspacing);
    QFETCH(QString, label);

    auto style = std::make_shared<ChartStyle>();
    style->setFontFamily(font);
    style->setTickLabelFontSize(tickfontsize);
    style->setLabelFontSize(lblfontsize);

    TraceChartHAxis hax(style);
    hax.setTickLength(ticklength);
    hax.setSpacings(labelspacing, ticklabelspacing, tickmarkspacing);
    hax.setLabel(label);

    TraceChartVAxis vax(style);
    vax.setTickLength(ticklength);
    vax.setSpacings(labelspacing, ticklabelspacing, tickmarkspacing);
    vax.setLabel(label);

    auto tickheight = QFontMetrics(QFont(font, tickfontsize)).height();
    auto tickwidth = QFontMetrics(QFont(font, tickfontsize)).width("0.9");
    auto labelheight = QFontMetrics(QFont(font, lblfontsize)).height();

    auto exp_h = tickheight + labelheight + ticklength + labelspacing + ticklabelspacing + tickmarkspacing;
    auto exp_v = tickwidth + labelheight + ticklength + labelspacing + ticklabelspacing + tickmarkspacing;
    QCOMPARE(hax.getThickness(), exp_h);
    QCOMPARE(vax.getThickness(), exp_v);

    hax.setVisible(false);
    vax.setVisible(false);
    QCOMPARE(hax.getThickness(), 0.);
    QCOMPARE(vax.getThickness(), 0.);
}

void tTraceChartWidget::tridgeline() {
    auto ridge = RidgeLineWidget();

    QCOMPARE(ridge.getYAxis()->getVisible(), false);
    QCOMPARE(ridge.getXAxis()->getVisible(), true);
    QCOMPARE(ridge.getXAxis()->getLabel(), "Time (s)");

    ridge.offset = 1.5;
    auto series1 = makeSeriesHelper(0, 1, { 0.f }, defstyle);
    ridge.addSeries(series1);
    auto series2 = makeSeriesHelper(0, 1, { 4.f }, defstyle);
    ridge.addSeries(series2);
    auto series3 = makeSeriesHelper(0, 1, { 4.f }, defstyle);
    ridge.addSeries(series3);
    ridge.updateOffsets();

    QCOMPARE(series1->getOffset(), 0.);
    QCOMPARE(series2->getOffset(), -1.5);
    QCOMPARE(series3->getOffset(), -3.);
}
// todo:
// unclear how to test paint in general, other than a snapshot
// saveasimage...feels like it might belong in fileio, needs a file fixture? But maybe this is the snapshot test?
// minimumsizehint...waiting for this until more progress on sizing and AR