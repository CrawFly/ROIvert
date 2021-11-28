#pragma once
#include "QtTest/QtTest"
#include "widgets/ColormapPickWidget.h"


class tColormapPickWidget : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void tColormapPickWidgetSetGet();
    void tColormapPickWidgetClick();
    
private:
    ColormapPickWidget* widget = nullptr;
};

