#include <QtTest/QtTest>
#include "tROIStyle.h"
#include "ROI/ROIStyle.h"

void tROIStyle::init() {
    style = new ROIStyle;
}
void tROIStyle::cleanup() {
    delete style;
    style = nullptr;
}

void tROIStyle::tpen() {
    int emitcntr = 0;
    connect(style, &ROIStyle::StyleChanged, [&](ROIStyle) {emitcntr++; });

    style->setColor(QColor(50, 150, 100));
    QCOMPARE(emitcntr, 1);
    style->setLineWidth(3);
    QCOMPARE(emitcntr, 2);

    QCOMPARE(style->getPen().color(), QColor(50, 150, 100));
    QCOMPARE(style->getPen().width(), 3);

    style->setLineColor(QColor(200, 50, 120));
    QCOMPARE(emitcntr, 3);
    QCOMPARE(style->getPen().color(), QColor(200, 50, 120));

    style->setLineWidth(0);
    QCOMPARE(emitcntr, 4);
    QCOMPARE(style->getPen().style(), Qt::PenStyle::NoPen);

    style->setLineWidth(1);
    QCOMPARE(emitcntr, 5);
    auto pen = style->getPen();
    QCOMPARE(style->getPen().style(),Qt::PenStyle::SolidLine);
}


void tROIStyle::tbrush() {
    int emitcntr = 0;
    connect(style, &ROIStyle::StyleChanged, [&](ROIStyle) {emitcntr++; });
    style->setColor(QColor(50, 150, 100));
    QCOMPARE(emitcntr, 1);

    style->setFillOpacity(255);
    QCOMPARE(emitcntr, 2);
    QCOMPARE(style->getBrush().style(), Qt::BrushStyle::SolidPattern);
    QCOMPARE(style->getBrush().color(), QColor(50, 150, 100));
    QCOMPARE(style->getBrush().color().alpha(), 255);

    style->setFillOpacity(128);
    QCOMPARE(emitcntr, 3);
    QCOMPARE(style->getBrush().color().alpha(), 128);
        
    style->setFillOpacity(0);
    QCOMPARE(emitcntr, 4);
    QCOMPARE(style->getBrush().style(), Qt::BrushStyle::NoBrush);
}
void tROIStyle::tselsize() {
    int emitcntr = 0;
    connect(style, &ROIStyle::StyleChanged, [&](ROIStyle) {emitcntr++; });
    style->setSelectorSize(14);
    QCOMPARE(emitcntr, 1);
    QCOMPARE(style->getSelectorSize(), 14);
}
void tROIStyle::tcolorbyselected() {
    int emitcntr = 0;
    connect(style, &ROIStyle::StyleChanged, [&](ROIStyle) {emitcntr++; });
    auto selclr = QColor(100, 20, 50);
    auto unselclr = QColor(50, 10, 70);
    style->setSelectedColor(selclr);
    QCOMPARE(emitcntr, 1);
    style->setUnselectedColor(unselclr);
    QCOMPARE(emitcntr, 2);
    style->setColorBySelected(true);
    QCOMPARE(emitcntr, 3);
    

    QVERIFY(style->isColorBySelected());
    style->setSelected(true);
    style->setFillOpacity(255);
    QCOMPARE(emitcntr, 5);
    QCOMPARE(style->getPen().color(), selclr);
    QCOMPARE(style->getBrush().color(), selclr);
    
    style->setSelected(false);
    QCOMPARE(emitcntr, 6);
    QCOMPARE(style->getPen().color(), unselclr);
    QCOMPARE(style->getBrush().color(), unselclr);
}
void tROIStyle::tpalettecolors() {
    ROIPalette p;
    
    // Note: signal must be used for MOC to generate, unused so untested
    //bool didemit = false;
    //connect(p.get(), &ROIPalette::paletteChanged, [&]{ didemit = true; });

    p.setPaletteColors({ Qt::red, Qt::green, Qt::blue });
    QCOMPARE(p.getPaletteColors(), std::vector<QColor>({ Qt::red, Qt::green, Qt::blue }));

    QCOMPARE(p.getPaletteColor(0), Qt::red);
    QCOMPARE(p.getPaletteColor(1), Qt::green);
    QCOMPARE(p.getPaletteColor(2), Qt::blue);
    QCOMPARE(p.getPaletteColor(13), Qt::green);


    auto act = p.getPaletteColors(std::vector<size_t>({ 0, 1, 2, 8, 7, 6 }));
    auto exp = std::vector<QColor>({ Qt::red, Qt::green, Qt::blue, Qt::blue, Qt::green, Qt::red });
    QCOMPARE(act, exp);

    // special case for empty palette, don't blow up!
    p.setPaletteColors(std::vector<QColor>());
    QCOMPARE(p.getPaletteColors(), std::vector<QColor>());
    QCOMPARE(p.getPaletteColors({ 0, 1 }), std::vector<QColor>(2));
    QCOMPARE(p.getPaletteColor({ 12 }), QColor());
}
void tROIStyle::tcopy() {
    style->setLineColor(Qt::cyan);
    style->setFillColor(Qt::magenta);
    style->setLineWidth(3);
    style->setFillOpacity(123);
    style->setSelectorSize(22);
    
    {
        auto cp = std::make_unique<ROIStyle>(*style);
        QCOMPARE(cp->getPen(), style->getPen());
        QCOMPARE(cp->getBrush(), style->getBrush());
        QCOMPARE(cp->getSelectorSize(), style->getSelectorSize());
    }
    {
        ROIStyle cp;
        cp = *style;
        QCOMPARE(cp.getPen(), style->getPen());
        QCOMPARE(cp.getBrush(), style->getBrush());
        QCOMPARE(cp.getSelectorSize(), style->getSelectorSize());
    }
}