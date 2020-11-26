#pragma once

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include "qboxlayout.h"
#include <QDebug>

using QtCharts::QLineSeries;
using QtCharts::QChartView;
using QtCharts::QChart;


class ChartViewClick : public QChartView
{
    Q_OBJECT

signals:
    void clicked();
    void keypressed(int key);

protected:
    void mousePressEvent(QMouseEvent* event) {
        /*QChart* chi = qobject_cast<QChart*>(chart());
        if (chi) {
            emit(clicked(chi->title()));
        }
        */
        emit(clicked());
    }
    void keyPressEvent(QKeyEvent* event) {
        emit keypressed(event->key());
    }
};

class TraceViewer : public QWidget
{
    Q_OBJECT

public:
    TraceViewer(QWidget *parent);
    ~TraceViewer();
    void setTrace(const int roiid, const std::vector<double> trace);
    void setSelectedTrace(int oldind, int newind);
signals:
    void chartClicked(int roiid);
    void deleteroi(int roiid);

public slots:
    void setmaxtime(double t_msecs);
    void roideleted(size_t roiind);

private:
    std::vector<QChart*> charts;
    std::vector<ChartViewClick*> chartviews;
    double maxtime = 1;

    void push_chart(int roiid);
    QColor selclr = QColor("#00CC66");
    QColor unselclr = QColor("#D90368");
    
    QVBoxLayout* lay;
    QScrollArea* scrollArea;
};