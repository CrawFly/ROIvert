#pragma once
#include <QWidget>

class ChartControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartControlWidget(QWidget* parent = nullptr);
    ~ChartControlWidget();

signals:
    void lineChartHeightChanged(int newheight);

public slots:
    void changeMinimumLineChartHeight(int minheight);
    void changeLineChartHeight(int newheight);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
