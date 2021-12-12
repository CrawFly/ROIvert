#pragma once
#include <QObject>

class ROIs;
class VideoData;
class ImageView;
class TraceViewWidget;

class tROIs : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void initTestCase();
    void cleanup();
    void cleanupTestCase();
    void taddroi();

private:
    ROIs* rois;
    VideoData* data;
    ImageView* iview;
    TraceViewWidget* tview;
};