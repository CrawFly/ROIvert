#include <QtTest>
#include <QCoreApplication>
#include "tChartStyle.h"
#include "tColormapPickWidget.h"

int main(int argc, char** argv)
{
    //QGuiApplication app(argc, argv);
    QApplication app(argc, argv);
    int status = 0;

    {
        tChartStyle testcase;
        status |= QTest::qExec(&testcase, argc, argv);
    }
    {
        tColormapPickWidget testcase;
        status |= QTest::qExec(&testcase, argc, argv);
    }

    qDebug() << (status == 0 ? "ALL TESTS PASSED" : "AT LEAST ONE TEST FAILURE");

   return status;
}