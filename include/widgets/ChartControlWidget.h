#pragma once
#include <QWidget>

class ChartControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartControlWidget(QWidget* parent = nullptr);
    ~ChartControlWidget();
    void setAutoTMax();

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
