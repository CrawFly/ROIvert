#pragma once
#include <QWidget>

class QVBoxLayout;
class TraceChartWidget;

class TraceView : public QWidget
{
    Q_OBJECT

public:
    TraceView(QWidget* parent = nullptr);
    ~TraceView();
    
    void setTimeLimits(float min, float max);
    
    //QVBoxLayout& getLineChartLayout();
    void addLineChart(TraceChartWidget*);

    TraceChartWidget& getRidgeChart();
    
private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};