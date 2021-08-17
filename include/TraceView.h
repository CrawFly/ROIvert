#pragma once
#include <QWidget>

class QVBoxLayout;
class TraceChartWidget;
class RidgeLineWidget;
class ChartStyle;

class TraceView : public QWidget
{
    Q_OBJECT

public:
    TraceView(QWidget* parent = nullptr);
    ~TraceView();
    
    void setTimeLimits(float min, float max);
    
    //QVBoxLayout& getLineChartLayout();
    void addLineChart(TraceChartWidget*);

    RidgeLineWidget& getRidgeChart() noexcept;

    ChartStyle& getCoreChartStyle();
    
private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};