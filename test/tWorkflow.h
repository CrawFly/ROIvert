#pragma once
#include <QObject>

class Roivert;

//** note: tWorkflow is not intended as a dumping ground for misc. tests that don't fit
//         but instead for integration tests that explicitly exercise:
//              - cross component behavior (e.g. switching df/f on|off)
//              - interactive workflows which are about mouse-work (e.g. drawing and editing ROIs)

class tWorkflow : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void troi();
private:
    Roivert* r;
};
