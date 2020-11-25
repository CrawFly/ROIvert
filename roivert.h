#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_roivert.h"
#include "imageroiviewer.h"
#include "videocontroller.h"
#include "opencv2/opencv.hpp"
#include "toolwindows.h"
#include "videodata.h"
#include "displaysettings.h"
#include "roivertcore.h"
//#include "tracecomputer.h"
#include "qthread.h"
#include "traceviewer.h"
// colors: (?)
//  2274A5
//  F75C03
//  F1C40F
//  D90368
//  00CC66



class Roivert : public QMainWindow
{
    Q_OBJECT

public:
    Roivert(QWidget *parent = Q_NULLPTR);
    ~Roivert() {
        //traceThread.quit();
        //traceThread.wait();
    }

public slots:
    void loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace);
    void changeFrame(const qint32 frame);
    void imgSettingsChanged(imgsettings settings);

signals:
    void MupdateTrace(ImageROIViewer*, VideoData*, const int);

private:
    Ui::RoivertClass ui;
    ImageROIViewer *imview;
    VideoController *vidctrl;
    VideoData* viddata;
    tool::imgData* t_imgData;
    tool::imgSettings* t_imgSettings;

    QDockWidget* w_imgData;
    QDockWidget* w_imgSettings;
    QDockWidget* w_charts;
    //TraceComputer* tcompute;
    DisplaySettings dispSettings;
    TraceViewer* tviewer;


    void frameRateChanged(double frameRate);
    void makeToolbar();
    void updateContrastWidget(bool isDff);
    void updateTrace(int roiid);

    QThread traceThread;


};
