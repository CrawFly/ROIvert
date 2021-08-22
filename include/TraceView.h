#pragma once
#include <QWidget>

class QVBoxLayout;
class TraceChartWidget;
class TraceChartSeries;
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
    
    ChartStyle* getCoreRidgeChartStyle() const noexcept;
    ChartStyle* getCoreLineChartStyle() const noexcept;
    
    void scrollToChart(TraceChartWidget*);


signals:
    void keyPressed(int key, Qt::KeyboardModifiers mod);
    void chartClicked(TraceChartWidget*, std::vector<TraceChartSeries*>, Qt::KeyboardModifiers);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};