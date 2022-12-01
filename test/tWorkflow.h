#pragma once
#include <QObject>

class Roivert;

//** note: tWorkflow is not intended as a dumping ground for misc. integration tests
//         but instead targets testing that really needs to simulate mouse activity,
//         or test relationships across components.

class tWorkflow : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void tload();
    void troi();
    void tzoom();
private:
    Roivert* r;
};
