#pragma once
#include <QWidget>
class TraceViewWidget;

class ChartControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartControlWidget(TraceViewWidget*);
    ~ChartControlWidget();
    void setAutoTMax();
    double getTMin() const;
    double getTMax() const;
    void setTRange(double min, double max);

signals:
    void lineChartHeightChanged(int newheight);
    void timeRangeChanged(double tmin, double tmax);
    

public slots:
    void changeMinimumLineChartHeight(int minheight);
    void changeLineChartHeight(int newheight);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
