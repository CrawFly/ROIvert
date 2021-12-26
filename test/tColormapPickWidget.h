#pragma once
#include <QObject>

class ColormapPickWidget;

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
