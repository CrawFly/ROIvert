#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_roivert.h"
#include "videocontroller.h"
#include "opencv2/opencv.hpp"
#include "toolwindows.h"
#include "videodata.h"
#include "displaysettings.h"
#include "roivertcore.h"
#include "ImageView.h"
#include "ROI/ROIs.h"
#include "FileIO.h"

class TraceView;


class Roivert : public QMainWindow
{
    Q_OBJECT

public:
    Roivert(QWidget *parent = Q_NULLPTR);

public slots:
    void loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace);
    void changeFrame(const size_t frame);
    void imgSettingsChanged(ROIVert::imgsettings settings);
    
protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void doConnect();


    Ui::RoivertClass ui;
    VideoController *vidctrl;
    VideoData* viddata;
    tool::imgData* t_imgData;
    tool::imgSettings* t_imgSettings;
    tool::fileIO* t_io;
    tool::colors* t_clrs;

    QDockWidget* w_imgData;
    QDockWidget* w_imgSettings;
    QDockWidget* w_io;
    QDockWidget* w_charts;
    QDockWidget* w_colors;
    

    DisplaySettings dispSettings;

    void frameRateChanged(double frameRate);
    void makeToolbar();
    void updateContrastWidget(bool isDff);
    void updateTrace(int roiid);

    void selecttoolfromkey(int key);

    void exportROIs(QString filename);
    void importROIs(QString filename);
    void restoreSettings();
    void resetLayout();

    cv::Mat TraceData;
    TraceView* traceview;
    ImageView* imageview;
    ROIs* rois;
    FileIO* fileio;
};
