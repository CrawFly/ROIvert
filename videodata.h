#pragma once

#include <QObject>

class VideoData : public QObject
{
    Q_OBJECT

public:
    VideoData(QObject *parent);
    ~VideoData();
};
