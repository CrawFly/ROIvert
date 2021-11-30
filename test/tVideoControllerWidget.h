#pragma once
#include <QObject>
#include "QtTest/QtTest"
#include "widgets/VideoControllerWidget.h"

class tVideoControllerWidget : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void tsetframe();

private:
    VideoControllerWidget* widget;
};

