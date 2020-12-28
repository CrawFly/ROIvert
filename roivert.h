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
#include "traceviewer.h"
#include <QtCharts/qvalueaxis.h>
using namespace QtCharts;




class Roivert : public QMainWindow
{
    Q_OBJECT

public:
    Roivert(QWidget *parent = Q_NULLPTR);

public slots:
    void loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace);
    void changeFrame(const size_t frame);
    void imgSettingsChanged(ROIVert::imgsettings settings);
    
signals:
    void MupdateTrace(ImageROIViewer*, VideoData*, const int);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::RoivertClass ui;
    ImageROIViewer *imview;
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
    TraceViewer* tviewer;


    void frameRateChanged(double frameRate);
    void makeToolbar();
    void updateContrastPickWidget(bool isDff);
    void updateTrace(int roiid);

    std::vector<std::vector<double>> traces;
    void selecttoolfromkey(int key);

    void exportTraces(QString filename, bool doHeader, bool doTimeCol);
    void exportROIs(QString filename);
    void importROIs(QString filename);
    void exportCharts(QString filename, bool doTitle, int width, int height);
    
    void restoreSettings();
    void resetLayout();
};
