#pragma once
#include "QtTest/QtTest"
#include "ChartStyle.h"

class tChartStyle: public QObject
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
private:
    ChartStyle* chartstyle = nullptr;
};
