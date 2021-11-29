#pragma once
#include "QtTest/QtTest"
#include "widgets/ColormapPickWidget.h"


class tColormapPickWidget : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void tSetGet();
    void tComboSelect();
    void tSignal();
    
private:
    ColormapPickWidget* widget = nullptr;
};

