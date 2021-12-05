#pragma once
#include <QObject>
#include "opencv2/opencv.hpp"


class VideoData;

class tVideoData : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void tload();
    void tproj();
    
private:
    VideoData* data;
    std::vector<cv::Mat> expraw;
    std::vector<cv::Mat> expdffi;
    cv::Mat expmeanraw;
    cv::Mat expmaxraw;
    cv::Mat expminraw;
    cv::Mat expmaxdff;
    cv::Mat expmindff;

    void loadmultipage(int dst=1, int dss=1);
    void loadjsonexpected();    
};


