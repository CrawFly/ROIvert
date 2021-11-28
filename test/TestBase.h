#include <QObject>
#include "QtTest/QtTest"

#pragma once
class TestBase: public QObject
{
    Q_OBJECT
public:
    TestBase() : executed(0), failed(0) {}
    std::pair<int, int> getExecAndFailed() {
        return { executed, failed };
    }

protected:
    void accumResults() {
        executed++;
        if (QTest::currentTestFailed()) {
            failed++;
        }
    }

private:
    int executed;
    int failed;
};