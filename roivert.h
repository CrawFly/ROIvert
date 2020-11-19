#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_roivert.h"
#include "imageroiviewer.h"
#include "videocontroller.h"
#include "opencv2/opencv.hpp"
#include "toolwindows.h"


// might as well make this a class and rename it or something, it grew...
struct videoMetaData {
    cv::Mat min,mind; //store a char and dbl version of each?
    cv::Mat max,maxd;
    cv::Mat mean,meand;

    int dsTime;
    int dsSpace;


    void init(const cv::Mat &image) {
        min = image.clone();
        max = image.clone();
        mean = cv::Mat(image.size(), image.type());
        sum = cv::Mat(image.size(), CV_64FC1);

        mind = cv::Mat(image.size(), CV_64FC1);
        maxd = cv::Mat(image.size(), CV_64FC1);
        meand = cv::Mat(image.size(), CV_64FC1);

        n = 0;
    }
    void accum(const cv::Mat& image) {
        min = cv::min(image, min);
        max = cv::max(image, max);
        cv::accumulate(image, sum);
        n++;
    }
    void complete() {
        sum.convertTo(mean, min.type(), 1./n);

        // store double versions of projections
        min.convertTo(mind, CV_64FC1);
        max.convertTo(maxd, CV_64FC1);
        mean.convertTo(meand, CV_64FC1);

        // store the range that df/f can occupy:
        cv::Mat mindff = dffd(mind);
        cv::Mat maxdff = dffd(maxd);
        cv::minMaxLoc(mindff, &dffminval, NULL);
        cv::minMaxLoc(maxdff, NULL, &dffmaxval);
        dffrng = dffmaxval - dffminval;
    }

    cv::Mat dff(cv::Mat frame) {
        // returns df/f of the frame as a uchar

        cv::Mat ret(frame.size(), CV_8UC1);
        // start with double 
        cv::Mat dffdbl = dffd(frame);
        
        // Convert to uchar, using normalization from range
        dffdbl.convertTo(ret, CV_8UC1, 255. / dffrng, -255 * dffminval / dffrng); 
        return ret;
    }

private:
    size_t n;
    cv::Mat sum;
    double dffminval;
    double dffmaxval;
    double dffrng;

    cv::Mat dffd(cv::Mat frame) {
        // calculates df/f double:
        cv::Mat ret(frame.size(), CV_64FC1);
        frame.convertTo(ret, CV_64FC1); // convert frame to double
        cv::subtract(ret, meand, ret);
        cv::divide(ret, meand, ret);
        return ret;
    }
};

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

    tool::imgData *t_img;

    std::vector<cv::Mat> *viddata = new std::vector<cv::Mat>;
    videoMetaData vidmeta;
};
