#include "tChartStyle.h"

#include <QtTest/QtTest>
#include "ChartStyle.h"

#include "ROI/ROIStyle.h"

void tChartStyle::init() {
    chartstyle = new ChartStyle;
}
void tChartStyle::cleanup() {
    delete chartstyle;
    chartstyle = nullptr;
}
void tChartStyle::tBackground()
{
    auto exp = QColor(123, 321, 69);
    chartstyle->setBackgroundColor(exp);
    auto act = chartstyle->getBackgroundColor();
    QCOMPARE(act, exp);
}
void tChartStyle::tFont() {
    chartstyle->setLabelFontSize(34);
    chartstyle->setTickLabelFontSize(23);
    chartstyle->setFontFamily("Calibri");

    auto lblfont = chartstyle->getLabelFont();
    auto tickfont = chartstyle->getTickLabelFont();
    QCOMPARE(lblfont.family(), "Calibri");
    QCOMPARE(tickfont.family(), "Calibri");
    QCOMPARE(lblfont.pointSize(), 34);
    QCOMPARE(tickfont.pointSize(), 23);

    auto lblmetrics = chartstyle->getLabelFontMetrics();
    auto tickmetrics = chartstyle->getTickLabelFontMetrics();

    QCOMPARE(lblmetrics.height(), QFontMetrics(QFont("Calibri", 34)).height());
    QCOMPARE(tickmetrics.height(), QFontMetrics(QFont("Calibri", 23)).height());
}
void tChartStyle::tLimits()
{
    chartstyle->setXLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
    QCOMPARE(chartstyle->getXLimitStyle(), ROIVert::LIMITSTYLE::MANAGED);
    chartstyle->setYLimitStyle(ROIVert::LIMITSTYLE::TIGHT);
    QCOMPARE(chartstyle->getYLimitStyle(), ROIVert::LIMITSTYLE::TIGHT);
}
void tChartStyle::tTracePen() {
    chartstyle->setTraceLineWidth(3);
    auto pen = chartstyle->getTracePen();
    QCOMPARE(pen.width(), 3);
    QCOMPARE(pen.style(), Qt::PenStyle::SolidLine);
    QCOMPARE(pen.isCosmetic(), true);

    chartstyle->setTraceLineWidth(0);
    pen = chartstyle->getTracePen();
    QCOMPARE(pen.style(), Qt::PenStyle::NoPen);

    chartstyle->setTraceLineWidth(2);
    pen = chartstyle->getTracePen();
    QCOMPARE(pen.width(), 2);
    QCOMPARE(pen.style(), Qt::PenStyle::SolidLine);
}
void tChartStyle::tBrush() {
    chartstyle->setTraceFillOpacity(123);
    chartstyle->setTraceFillGradient(true);
    chartstyle->setDoBackBrush(true);
    auto brush = chartstyle->getTraceBrush();
    QCOMPARE(brush.color().alpha(), 123);
    QCOMPARE(brush.style(), Qt::BrushStyle::SolidPattern);
    QCOMPARE(chartstyle->getDoBackBrush(), true);
    QCOMPARE(chartstyle->getTraceFillGradient(), true);

    chartstyle->setTraceFillOpacity(0);
    brush = chartstyle->getTraceBrush();
    QCOMPARE(brush.style(), Qt::BrushStyle::NoBrush);

    chartstyle->setTraceFillOpacity(200);
    brush = chartstyle->getTraceBrush();
    QCOMPARE(brush.color().alpha(), 200);
    QCOMPARE(brush.style(), Qt::BrushStyle::SolidPattern);
}
void tChartStyle::tAxis() {
    chartstyle->setAxisColor(QColor(255, 123, 42));
    chartstyle->setAxisLineWidth(4);
    chartstyle->setGrid(true);
    QCOMPARE(chartstyle->getGrid(), true);

    auto pen = chartstyle->getAxisPen();
    QCOMPARE(pen.width(), 4);
    QCOMPARE(pen.color(), QColor(255, 123, 42));
}
void tChartStyle::tROIStyleLink() {
    ROIStyle roistyle;
    chartstyle->connectToROIStyle(&roistyle);
    chartstyle->setTraceLineWidth(1);
    chartstyle->setTraceFillOpacity(255);

    roistyle.setLineColor(QColor(11, 22, 33));
    roistyle.setFillColor(QColor(44, 55, 66));

    auto pen = chartstyle->getTracePen();
    auto brush = chartstyle->getTraceBrush();

    QCOMPARE(pen.color(), QColor(11, 22, 33));
    QCOMPARE(brush.color(), QColor(44, 55, 66));
}
void tChartStyle::tNormalization() {
    chartstyle->setNormalization(ROIVert::NORMALIZATION::ZSCORE);
    QCOMPARE(chartstyle->getNormalization(), ROIVert::NORMALIZATION::ZSCORE);
}
void tChartStyle::tCopy() {
    chartstyle->setBackgroundColor(QColor(123, 321, 69));
    chartstyle->setLabelFontSize(34);
    chartstyle->setTickLabelFontSize(23);
    chartstyle->setFontFamily("Calibri");
    chartstyle->setXLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
    chartstyle->setYLimitStyle(ROIVert::LIMITSTYLE::TIGHT);
    chartstyle->setTraceLineWidth(3);
    chartstyle->setTraceFillOpacity(123);
    chartstyle->setTraceFillGradient(true);
    chartstyle->setDoBackBrush(true);
    chartstyle->setAxisColor(QColor(255, 123, 42));
    chartstyle->setAxisLineWidth(4);
    chartstyle->setGrid(true);
    chartstyle->setNormalization(ROIVert::NORMALIZATION::ZSCORE);
    {
        auto cp = std::make_unique<ChartStyle>(*chartstyle);
        QCOMPARE(cp->getBackgroundColor(), chartstyle->getBackgroundColor());
        QCOMPARE(cp->getLabelFont(), chartstyle->getLabelFont());
        QCOMPARE(cp->getTickLabelFont(), chartstyle->getTickLabelFont());
        QCOMPARE(cp->getXLimitStyle(), chartstyle->getXLimitStyle());
        QCOMPARE(cp->getYLimitStyle(), chartstyle->getYLimitStyle());
        QCOMPARE(cp->getTracePen(), chartstyle->getTracePen());
        QCOMPARE(cp->getTraceBrush(), chartstyle->getTraceBrush());
        QCOMPARE(cp->getGrid(), chartstyle->getGrid());
        QCOMPARE(cp->getAxisPen(), chartstyle->getAxisPen());
        QCOMPARE(cp->getNormalization(), chartstyle->getNormalization());
    }

    {
        ChartStyle cp;
        cp = *chartstyle;
        QCOMPARE(cp.getBackgroundColor(), chartstyle->getBackgroundColor());
        QCOMPARE(cp.getLabelFont(), chartstyle->getLabelFont());
        QCOMPARE(cp.getTickLabelFont(), chartstyle->getTickLabelFont());
        QCOMPARE(cp.getXLimitStyle(), chartstyle->getXLimitStyle());
        QCOMPARE(cp.getYLimitStyle(), chartstyle->getYLimitStyle());
        QCOMPARE(cp.getTracePen(), chartstyle->getTracePen());
        QCOMPARE(cp.getTraceBrush(), chartstyle->getTraceBrush());
        QCOMPARE(cp.getGrid(), chartstyle->getGrid());
        QCOMPARE(cp.getAxisPen(), chartstyle->getAxisPen());
        QCOMPARE(cp.getNormalization(), chartstyle->getNormalization());
    }
}