#pragma once
#include <QDockWidget>
#include <QScrollArea>

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

    void addLineChart(TraceChartWidget*);
    RidgeLineWidget& getRidgeChart() noexcept;

    ChartStyle* getCoreRidgeChartStyle() const noexcept;
    ChartStyle* getCoreLineChartStyle() const noexcept;

    void scrollToChart(TraceChartWidget*);
    double makeAllTimeLimitsAuto();
    void updateTMax(float tmax);

signals:
    void keyPressed(int key, Qt::KeyboardModifiers mod);
    void chartClicked(TraceChartWidget*, std::vector<TraceChartSeries*>, Qt::KeyboardModifiers);

public slots:
    void updateMinimumHeight();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};

// RScrollArea is a little wrapper around QScrollArea to let modified scoll perform
// a differnt behavior (to resize the charts)
class RScrollArea : public QScrollArea
{
    Q_OBJECT

public:
signals:
    void modwheel(int delta);

protected:
    void wheelEvent(QWheelEvent* event) override;
};