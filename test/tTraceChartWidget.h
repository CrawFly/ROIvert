#pragma once
#include <QObject>
class TraceChartWidget;
class TraceChartAxis;
class ChartStyle;

class tTraceChartWidget : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void tstyle();
    void tnormalization_data();
    void tnormalization();
    void taddremoveseries();
    void ttitle();
    void tantialiasing();
    void tclick();

    void tseriesdata();
    void tseriesextents();
    void tseriesoffset();
    void tseriesdegendata();
    void tseriessetstyle();

    void taxislimits();
    void taxisticks();
    void taxisthickness_data();
    void taxisthickness();

    void tridgeline();
private:
    TraceChartWidget* chart;
    TraceChartAxis* xaxis;
    TraceChartAxis* yaxis;
    std::shared_ptr<ChartStyle> defstyle;
};