#pragma once
#include <QObject>

class Roivert;
class StyleWidget;
class ROIStyle;
class ChartStyle;


class tStyleWidget : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void tROIColor();
    void tROIStyle();
    void tChartFonts();
    void tLineStyle();
    void tRidgeStyle();

private:
    Roivert* r;
    StyleWidget* s;
    void addROI();
    std::vector<ROIStyle*> getROIStyles();
    std::vector<ChartStyle*> getChartStyles();
};
