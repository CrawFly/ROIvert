#pragma once
#include <QObject>

class TraceViewWidget;
class ChartControlWidget;

class tTraceViewWidget : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void tChartControlsTimeRange();
    void tChartControlsChartHeight();
    void tScrollToChart();

private:
    TraceViewWidget* tview = nullptr;
    ChartControlWidget* chartcontrols = nullptr;
};
