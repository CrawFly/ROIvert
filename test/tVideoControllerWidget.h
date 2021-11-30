#pragma once
#include <QObject>

class VideoControllerWidget;

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

