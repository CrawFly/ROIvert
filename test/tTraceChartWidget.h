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

private:
    TraceChartWidget* chart;
    TraceChartAxis* xaxis;
    TraceChartAxis* yaxis;
    std::shared_ptr<ChartStyle> defstyle;
};