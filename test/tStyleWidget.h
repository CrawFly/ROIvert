#pragma once
#include <QObject>

class Roivert;
class StyleWidget;
class ROIStyle;
class ChartStyle;
class ROIs;

class tStyleWidget : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void tROIColor();
    void tROIStyle();
    void tChartColors();
    void tChartFonts();
    void tLineStyle();
    void tRidgeStyle();

private:
    Roivert* r;
    StyleWidget* s;
    ROIs* rois;

    void addROI();
    std::vector<ROIStyle*> getROIStyles();
    std::vector<ChartStyle*> getLineChartStyles();

    ChartStyle* getRidgeChartStyle();
};
