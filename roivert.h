#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_roivert.h"
#include "imageroiviewer.h"
#include "videocontroller.h"
#include "opencv2/opencv.hpp"
#include "toolwindows.h"
#include "videodata.h"

class Roivert : public QMainWindow
{
    Q_OBJECT

public:
    Roivert(QWidget *parent = Q_NULLPTR);

public slots:
    void loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace);
    void changeFrame(const qint32 frame);

private:
    Ui::RoivertClass ui;
    ImageROIViewer *imview;
    VideoController *vidctrl;
    VideoData* viddata;
    tool::imgData* t_img;

    void frameRateChanged(double frameRate);
};
