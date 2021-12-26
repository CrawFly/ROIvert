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
    void tload_data();
    void tproj();
    void tdowns();
    void tdowns_data();
    void tdownt();
    void tdownt_data();
    void tfr();
    void ttrace();
    void tdeadpixel();

    void tmultifile();
    void tmultifile_data();

    void temptyfilelist();
    void tgetoverflow();
    void tnowidthtrace();

    void thistogram();
private:
    VideoData* data;
    std::vector<cv::Mat> expraw;
    std::vector<cv::Mat> expdffi;
    std::vector<cv::Mat> deadpix;
    cv::Mat expmeanraw;
    cv::Mat expmaxraw;
    cv::Mat expminraw;
    cv::Mat expmaxdff;
    cv::Mat expmindff;
    std::vector<float> roiwholefield;
    std::vector<float> roiellipse;
    std::vector<float> roitril;

    void loadmultipage(int dst = 1, int dss = 1);
    void loadjsonexpected();
};

//todo:
//  16 bit datasets
