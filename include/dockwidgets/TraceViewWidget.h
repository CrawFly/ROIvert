#pragma once
#include <QDockWidget>

class QVBoxLayout;
class TraceChartWidget;
class TraceChartSeries;
class RidgeLineWidget;
class ChartStyle;

class TraceViewWidget : public QDockWidget
{
    Q_OBJECT

public:
    TraceViewWidget(QWidget* parent = nullptr);
    ~TraceViewWidget();
    
    
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