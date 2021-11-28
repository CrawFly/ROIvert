#include "tColormapPickWidget.h"

void tColormapPickWidget::init() {
    widget = new ColormapPickWidget;
}
void tColormapPickWidget::cleanup() {
    delete widget;
    widget = nullptr;
}

void tColormapPickWidget::tColormapPickWidgetSetGet() {
    widget->setColormap(21);
    QCOMPARE(widget->getColormap(), 21);

    widget->setColormap(-1);
    QCOMPARE(widget->getColormap(), -1);

    widget->setColormap(11);
    QCOMPARE(widget->getColormap(), 11);

}
void tColormapPickWidget::tColormapPickWidgetClick() {
    auto cmb = widget->findChild<QComboBox*>("cmbColormap");
    cmb->setCurrentIndex(1);
    QCOMPARE(widget->getColormap(), 21);
    cmb->setCurrentIndex(0);
    QCOMPARE(widget->getColormap(), -1);
    cmb->setCurrentIndex(2);
    QCOMPARE(widget->getColormap(), 11);
}