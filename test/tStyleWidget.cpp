#include "tStyleWidget.h"
#include <QtTest/QtTest>
#include "testUtils.h"
#include "roivert.h"
#include "dockwidgets/StyleWidget.h"
#include "ROI/ROIStyle.h"
#include "ChartStyle.h"
#include "widgets/RGBWidget.h"


#include <QSlider>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>

void tStyleWidget::init() {
    r = new Roivert;
    QVERIFY(r);
    s = r->findChild<StyleWidget*>();
    QVERIFY(s);
}
void tStyleWidget::cleanup() {
    delete(r);
}
void tStyleWidget::addROI(){}
std::vector<ROIStyle*> tStyleWidget::getROIStyles() {
    std::vector<ROIStyle*> ret;

    return ret;
}
std::vector<ChartStyle*> tStyleWidget::getChartStyles() {
    std::vector<ChartStyle*> ret;

    return ret;
}

void tStyleWidget::tROIColor()
{

}
void tStyleWidget::tROIStyle()
{

}
void tStyleWidget::tChartFonts() {}
void tStyleWidget::tLineStyle(){}
void tStyleWidget::tRidgeStyle() {}
