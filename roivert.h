#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_roivert.h"
#include "widgets/VideoControllerWidget.h"
#include "opencv2/opencv.hpp"
#include "videodata.h"
#include "DisplaySettings.h"
#include "roivertcore.h"
#include "ImageView.h"
#include "ROI/ROIs.h"
#include "FileIO.h"

#include "dockwidgets/StyleWidget.h"
#include "dockwidgets/FileIOWidget.h"
#include "dockwidgets/ImageSettingsWidget.h"
#include "dockwidgets/ImageDataWidget.h"

class TraceView;
class QStringList;

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
    VideoControllerWidget *vidctrl;
    VideoData* viddata;
    
    ImageDataWidget* imagedatawidget;
    StyleWidget* stylewidget;
    FileIOWidget* fileiowidget;
    ImageSettingsWidget* imagesettingswidget;

    QDockWidget* w_charts;

    DisplaySettings dispSettings;

    void frameRateChanged(double frameRate);
    void makeToolbar();
    void updateContrastWidget(bool isDff);
    void selecttoolfromkey(int key);
    void restoreSettings();
    void resetSettings();

    cv::Mat TraceData;
    TraceView* traceview;
    ImageView* imageview;
    ROIs* rois;
    FileIO* fileio;
};
