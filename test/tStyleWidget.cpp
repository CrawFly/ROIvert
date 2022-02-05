#include "tStyleWidget.h"
#include <QtTest/QtTest>
#include "testUtils.h"
#include "roivert.h"
#include "dockwidgets/StyleWidget.h"
#include "ROI/ROIStyle.h"
#include "ROI/ROIs.h"
#include "ChartStyle.h"
#include "widgets/RGBWidget.h"
#include "dockwidgets/TraceViewWidget.h"
#include "VideoData.h"
#include "widgets/TraceChartWidget.h"

#include <QSlider>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QColor>

void tStyleWidget::init() {
    r = new Roivert;
    QVERIFY(r);
    s = r->findChild<StyleWidget*>();
    QVERIFY(s);

    rois = r->findChild<ROIs*>();
    QVERIFY(rois);

    auto v = r->findChild<VideoData*>();
    QVERIFY(v);
    loaddataset(v);

    
}
void tStyleWidget::cleanup() {
    delete(r);
}

void tStyleWidget::addROI()
{
    rois->pushROI(QPoint(1, 1), ROIVert::SHAPE::RECTANGLE);
    auto ind = rois->size() - 1;
    auto& shp = (*rois)[ind].graphicsShape;
    shp->setVertices({ QPoint(1,1), QPoint(3,3) });
    (*rois)[0].Trace->updateTrace(ROIVert::SHAPE::RECTANGLE, shp->getTightBoundingBox(), shp->getVertices());    
}
std::vector<ROIStyle*> tStyleWidget::getROIStyles() 
{
    std::vector<ROIStyle*> ret(rois->size());
    for (size_t i = 0; i < rois->size(); ++i) {
        ret[i] = (*rois)[i].roistyle.get();
    }
    return ret;
}
std::vector<ChartStyle*> tStyleWidget::getLineChartStyles() {
    std::vector<ChartStyle*> ret(rois->size());
    
    for (size_t i = 0; i < rois->size(); ++i) {
        ret[i] = (*rois)[i].linechartstyle.get();
    }
    

    return ret;
}



ChartStyle* tStyleWidget::getRidgeChartStyle() {
    
    auto tview = r->findChild<TraceViewWidget*>();
    return tview->getRidgeChart().getStyle();
}


void tStyleWidget::tROIColor()
{
    auto roicolor = s->findChild<RGBWidget*>("roicolor");
    QVERIFY(roicolor);
    
    addROI();
    addROI();
    auto clr0 = QColor(12, 34, 56);
    auto clr1 = QColor(78, 90, 12);
    auto clr2 = QColor(34, 56, 78);

    rois->setSelected({ 0 });
    roicolor->setColor(clr0);
    rois->setSelected({ 1 });
    roicolor->setColor(clr1);

    QCOMPARE(getROIStyles()[0]->getLineColor(), clr0);
    QCOMPARE(getROIStyles()[1]->getLineColor(), clr1);
    QCOMPARE(getLineChartStyles()[0]->getTracePen().color(), clr0);
    QCOMPARE(getLineChartStyles()[1]->getTracePen().color(), clr1);

    
    rois->setSelected({ 0, 1 });
    roicolor->setColor(clr2);
    QCOMPARE(getROIStyles()[0]->getLineColor(), clr2);
    QCOMPARE(getROIStyles()[1]->getLineColor(), clr2);
    QCOMPARE(getLineChartStyles()[0]->getTracePen().color(), clr2);
    QCOMPARE(getLineChartStyles()[1]->getTracePen().color(), clr2);
}
void tStyleWidget::tROIStyle()
{
    auto roilinewidth = s->findChild<QSlider*>("roilinewidth");
    auto roiselsize = s->findChild<QSlider*>("roiselsize");
    auto roifillopacity = s->findChild<QSlider*>("roifillopacity");
    QVERIFY(roilinewidth);
    QVERIFY(roiselsize);
    QVERIFY(roifillopacity);

    // note that style controls should affect all existing, as well as all new rois
    addROI();
    addROI();
    roilinewidth->setValue(4);
    roiselsize->setValue(5);
    roifillopacity->setValue(200);

    auto rss = getROIStyles();
    for (auto& rs : rss) {
        QCOMPARE(rs->getPen().width(), 4);
        QCOMPARE(rs->getSelectorSize(), 5);
        QCOMPARE(rs->getBrush().color().alpha(), 200);
    }

    
    addROI();
    addROI();
    rss = getROIStyles();
    for (auto& rs : rss) {
        QCOMPARE(rs->getPen().width(), 4);
        QCOMPARE(rs->getSelectorSize(), 5);
        QCOMPARE(rs->getBrush().color().alpha(), 200);
    }
}
void tStyleWidget::tChartColors() 
{
    auto chartforecolor = s->findChild<RGBWidget*>("chartforecolor");
    auto chartbackcolor = s->findChild<RGBWidget*>("chartbackcolor");
    auto blackfore = s->findChild<QToolButton*>("blackfore");
    auto whitefore = s->findChild<QToolButton*>("whitefore");
    auto blackback = s->findChild<QToolButton*>("blackback");
    auto whiteback = s->findChild<QToolButton*>("whiteback");
    QVERIFY(chartforecolor);
    QVERIFY(chartbackcolor);
    QVERIFY(blackfore);
    QVERIFY(whitefore);
    QVERIFY(blackback);
    QVERIFY(whiteback);
    
    addROI();
    addROI();
    QColor fcolor(12, 34, 56);
    QColor bcolor(112, 134, 156);
    
    { //custom fore and back
        chartforecolor->setColor(fcolor);
        chartbackcolor->setColor(bcolor);
        auto css = getLineChartStyles();
        for (auto& cs : css) {
            QCOMPARE(cs->getAxisPen().color(), fcolor);
            QCOMPARE(cs->getBackgroundColor(), bcolor);
        }
        QCOMPARE(getRidgeChartStyle()->getAxisPen().color(), fcolor);
        QCOMPARE(getRidgeChartStyle()->getBackgroundColor(), bcolor);

    }
    { // black fore, and post color add
        blackfore->clicked();
        QCOMPARE(chartforecolor->getColor(), Qt::black);
        QCOMPARE(chartbackcolor->getColor(), bcolor);
        addROI();
        auto css = getLineChartStyles();
        for (auto& cs : css) {
            QCOMPARE(cs->getAxisPen().color(), Qt::black);
            QCOMPARE(cs->getBackgroundColor(), bcolor);
        }
        QCOMPARE(getRidgeChartStyle()->getAxisPen().color(), Qt::black);
        QCOMPARE(getRidgeChartStyle()->getBackgroundColor(), bcolor);
    }
    { // white fore, and post color add
        whitefore->clicked();
        QCOMPARE(chartforecolor->getColor(), Qt::white);
        QCOMPARE(chartbackcolor->getColor(), bcolor);
        addROI();
        auto css = getLineChartStyles();
        for (auto& cs : css) {
            QCOMPARE(cs->getAxisPen().color(), Qt::white);
            QCOMPARE(cs->getBackgroundColor(), bcolor);
        }
        QCOMPARE(getRidgeChartStyle()->getAxisPen().color(), Qt::white);
        QCOMPARE(getRidgeChartStyle()->getBackgroundColor(), bcolor);
    }
    
    { // black back, and post color add
        blackback->clicked();
        QCOMPARE(chartforecolor->getColor(), Qt::white);
        QCOMPARE(chartbackcolor->getColor(), Qt::black);
        addROI();
        auto css = getLineChartStyles();
        for (auto& cs : css) {
            QCOMPARE(cs->getAxisPen().color(), Qt::white);
            QCOMPARE(cs->getBackgroundColor(), Qt::black);
        }
        QCOMPARE(getRidgeChartStyle()->getAxisPen().color(), Qt::white);
        QCOMPARE(getRidgeChartStyle()->getBackgroundColor(), Qt::black);
    }
    { // white back, and post color add
        whiteback->clicked();
        QCOMPARE(chartforecolor->getColor(), Qt::white);
        QCOMPARE(chartbackcolor->getColor(), Qt::white);
        addROI();
        auto css = getLineChartStyles();
        for (auto& cs : css) {
            QCOMPARE(cs->getAxisPen().color(), Qt::white);
            QCOMPARE(cs->getBackgroundColor(), Qt::white);
        }
        QCOMPARE(getRidgeChartStyle()->getAxisPen().color(), Qt::white);
        QCOMPARE(getRidgeChartStyle()->getBackgroundColor(), Qt::white);
    }

}
void tStyleWidget::tChartFonts() 
{
    auto chartfont = s->findChild<QComboBox*>("chartfont");
    auto chartlabelfontsize = s->findChild<QSpinBox*>("chartlabelfontsize");
    auto charttickfontsize = s->findChild<QSpinBox*>("charttickfontsize");
    QVERIFY(chartfont);
    QVERIFY(chartlabelfontsize);
    QVERIFY(charttickfontsize);

    addROI();
    addROI();

    auto fontind = chartfont->count() - 1;
    QVERIFY(fontind != chartfont->currentIndex() && fontind > 0);
    chartfont->setCurrentIndex(fontind);
    chartfont->currentIndexChanged(fontind);
    chartlabelfontsize->setValue(22);
    charttickfontsize->setValue(23);
    
    auto css = getLineChartStyles();
    for (auto& cs : css) {
        QCOMPARE(cs->getLabelFont().family(), chartfont->currentText());
        QCOMPARE(cs->getLabelFont().pointSize(), 22);
        QCOMPARE(cs->getTickLabelFont().pointSize(), 23);
    }
    QCOMPARE(getRidgeChartStyle()->getLabelFont().pointSize(), 22);
    QCOMPARE(getRidgeChartStyle()->getTickLabelFont().pointSize(), 23);
    QCOMPARE(getRidgeChartStyle()->getLabelFont().family(), chartfont->currentText());
    
    addROI();
    addROI();
    css = getLineChartStyles();
    for (auto& cs : css) {
        QCOMPARE(cs->getLabelFont().family(), chartfont->currentText());
        QCOMPARE(cs->getLabelFont().pointSize(), 22);
        QCOMPARE(cs->getTickLabelFont().pointSize(), 23);
    }
    QCOMPARE(getRidgeChartStyle()->getLabelFont().pointSize(), 22);
    QCOMPARE(getRidgeChartStyle()->getTickLabelFont().pointSize(), 23);
    QCOMPARE(getRidgeChartStyle()->getLabelFont().family(), chartfont->currentText());


}
void tStyleWidget::tLineStyle()
{
    auto linewidth = s->findChild<QSpinBox*>("linewidth");
    auto linefill = s->findChild<QSlider*>("linefill");
    auto linegradient = s->findChild<QCheckBox*>("linegradient");
    auto linegrid = s->findChild<QCheckBox*>("linegrid");
    auto linematchy = s->findChild<QCheckBox*>("linematchy");
    auto linenorm = s->findChild<QComboBox*>("linenorm");
    

    QVERIFY(linewidth);
    QVERIFY(linefill);
    QVERIFY(linegradient);
    QVERIFY(linegrid);
    QVERIFY(linematchy);
    QVERIFY(linenorm);

    addROI();
    addROI();
    linewidth->setValue(3);
    linefill->setValue(120);
    linegradient->setChecked(true);
    linegrid->setChecked(true);
    linematchy->setChecked(true);
    linenorm->setCurrentIndex(5);
    auto css = getLineChartStyles();
    for (auto& cs : css) {
        QCOMPARE(cs->getTracePen().width(), 3);
        QCOMPARE(cs->getTraceBrush().color().alpha(), 120);
        QCOMPARE(cs->getTraceFillGradient(), true);
        QCOMPARE(cs->getGrid(), true);
        QCOMPARE(cs->getYLimitStyle(), ROIVert::LIMITSTYLE::MANAGED);
        QCOMPARE(cs->getNormalization(), ROIVert::NORMALIZATION::MEDIQR);
    }
    addROI();
    addROI();

    rois->update(); // this happens automatically outside of test.
    css = getLineChartStyles();
    for (auto& cs : css) {
        QCOMPARE(cs->getTracePen().width(), 3);
        QCOMPARE(cs->getTraceBrush().color().alpha(), 120);
        QCOMPARE(cs->getTraceFillGradient(), true);
        QCOMPARE(cs->getGrid(), true);
        QCOMPARE(cs->getYLimitStyle(), ROIVert::LIMITSTYLE::MANAGED);
        QCOMPARE(cs->getNormalization(), ROIVert::NORMALIZATION::MEDIQR);
    }

}
void tStyleWidget::tRidgeStyle() 
{
    auto ridgewidth = s->findChild<QSpinBox*>("ridgewidth");
    auto ridgefill = s->findChild<QSlider*>("ridgefill");

    auto ridgeoverlap = s->findChild<QSlider*>("ridgeoverlap");
    auto ridgegradient = s->findChild<QCheckBox*>("ridgegradient");
    auto ridgegrid = s->findChild<QCheckBox*>("ridgegrid");
    QVERIFY(ridgewidth);
    QVERIFY(ridgefill);
    QVERIFY(ridgeoverlap);
    QVERIFY(ridgegradient);
    QVERIFY(ridgegrid);
    
    auto tview = r->findChild<TraceViewWidget*>();
    auto& ridgechart = tview->getRidgeChart();
    
    
    addROI();
    addROI();
    ridgewidth->setValue(3);
    ridgefill->setValue(120);
    ridgegradient->setChecked(true);
    ridgegrid->setChecked(true);
    ridgeoverlap->setValue(22);

    QCOMPARE(getRidgeChartStyle()->getTracePen().width(), 3);
    QCOMPARE(getRidgeChartStyle()->getTraceBrush().color().alpha(), 120);
    QCOMPARE(getRidgeChartStyle()->getTraceFillGradient(), true);
    QCOMPARE(getRidgeChartStyle()->getGrid(), true);
    {
        const auto& ridgeser = ridgechart.getSeries();
        QCOMPARE(ridgeser.size(), 2);
        QCOMPARE(ridgeser[0]->getOffset(), 0.f);
        QCOMPARE(ridgeser[1]->getOffset(), -.22f);
    }
    


    addROI();
    addROI();

    QCOMPARE(getRidgeChartStyle()->getTracePen().width(), 3);
    QCOMPARE(getRidgeChartStyle()->getTraceBrush().color().alpha(), 120);
    QCOMPARE(getRidgeChartStyle()->getTraceFillGradient(), true);
    QCOMPARE(getRidgeChartStyle()->getGrid(), true);
    {
        const auto& ridgeser = ridgechart.getSeries();
        QCOMPARE(ridgeser.size(), 4);
        QCOMPARE(ridgeser[0]->getOffset(), 0.f);
        QCOMPARE(ridgeser[1]->getOffset(), -.22f);
        QCOMPARE(ridgeser[2]->getOffset(), -.44f);
        QCOMPARE(ridgeser[3]->getOffset(), -.66f);
    }

}
