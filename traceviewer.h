#pragma once

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include "qboxlayout.h"


using QtCharts::QLineSeries;
using QtCharts::QChartView;
using QtCharts::QChart;


class TraceViewer : public QWidget
{
    Q_OBJECT

public:
    TraceViewer(QWidget *parent);
    ~TraceViewer();
    void setTrace(const int roiid, const std::vector<double> trace);

public slots:
    void setmaxtime(float t_msecs);

private:
    std::vector<QChart*> charts;
    std::vector<QChartView*> chartviews;

    QVBoxLayout* lay;
    QScrollArea* scrollArea;
};
