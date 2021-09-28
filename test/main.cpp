//#include <QtTest/QtTest>
#include "test.h"

void TestQString::toUpper()
{
    //QVERIFY(true);
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HsELLO"));
}
QTEST_MAIN(TestQString)