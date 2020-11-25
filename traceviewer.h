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
    void setSelectedTrace(int oldind, int newind);

public slots:
    void setmaxtime(double t_msecs);

private:
    std::vector<QChart*> charts;
    std::vector<QChartView*> chartviews;
    double maxtime = 1;

    void push_chart(int roiid);
    QColor selclr = QColor("#00CC66");
    QColor unselclr = QColor("#D90368");
    
    QVBoxLayout* lay;
    QScrollArea* scrollArea;
};