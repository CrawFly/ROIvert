#pragma once
#include <QObject>
class ContrastWidget;

class tContrastWidget : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void tsetgetcontrast();
    void tsetspinners();
    void thistogram();
    void tlinecolor();
    void tchart2spin();
    void tsignal();

private:
    ContrastWidget* widget = nullptr;

    struct implptrs;
    implptrs* ptrs;
};
