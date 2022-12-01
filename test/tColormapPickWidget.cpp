#include "tColormapPickWidget.h"
#include "widgets/ColormapPickWidget.h"
#include <QtTest/QtTest>

void tColormapPickWidget::init() {
    widget = new ColormapPickWidget;
}
void tColormapPickWidget::cleanup() {
    delete widget;
    widget = nullptr;
}

void tColormapPickWidget::tSetGet() {
    widget->setColormap(21);
    QCOMPARE(widget->getColormap(), 21);

    widget->setColormap(-1);
    QCOMPARE(widget->getColormap(), -1);

    widget->setColormap(11);
    QCOMPARE(widget->getColormap(), 11);
}
void tColormapPickWidget::tComboSelect() {
    auto cmb = widget->findChild<QComboBox*>("cmbColormap");
    cmb->setCurrentIndex(1);
    QCOMPARE(widget->getColormap(), 21);
    cmb->setCurrentIndex(0);
    QCOMPARE(widget->getColormap(), -1);
    cmb->setCurrentIndex(2);
    QCOMPARE(widget->getColormap(), 11);
}

void tColormapPickWidget::tSignal() {
    // this tests that the widget emits when the combo is activated...
    bool didsend = false;
    connect(widget, &ColormapPickWidget::colormapChanged, [&]() { didsend = true; });
    auto cmb = widget->findChild<QComboBox*>("cmbColormap");
    emit(cmb->activated(1));
    QVERIFY(didsend);
}