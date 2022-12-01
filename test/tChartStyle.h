#pragma once
#include <QObject>

class ChartStyle;

class tChartStyle : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void tBackground();
    void tFont();
    void tLimits();
    void tTracePen();
    void tBrush();
    void tAxis();
    void tROIStyleLink();
    void tNormalization();

    void tCopy();
private:
    ChartStyle* chartstyle = nullptr;
};
