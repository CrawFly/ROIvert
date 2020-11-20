#pragma once

#include <QObject>
#include <QStringList>
#include "opencv2/opencv.hpp"

class VideoData : public QObject
{
    Q_OBJECT

public:
    enum class projection
    {
        MIN,
        MAX,
        MEAN,
        SUM
    };

    VideoData(QObject *parent);
    ~VideoData();

    void load(QStringList filelist, int dst, int dss);
    cv::Mat getFrameRaw(size_t frameindex);
    cv::Mat getFrameDff(size_t frameindex);
    cv::Mat getProjection(VideoData::projection proj);

    void setStoreDff(bool enabled);
    bool getStoreDff();
    
    //cv::Mat getFrameHistogramRaw(size_t frameindex); 
    //cv::Mat getFrameHistogramDff(size_t frameindex);
    //cv::Mat getHistogramRaw(size_t frameindex);
    //cv::Mat getHistogramDff(size_t frameindex);

    int getWidth();
    int getHeight();
    size_t getNFrames();

signals:
    void frameLoaded(size_t frameNumber); // fires whether it's valid or not
    void loadComplete(size_t nframes, int height, int width, int ndropped);

private:
    bool storeDff = true;

    QStringList files;
    int width=0, height=0, nframes=0;

    // Projections:
    cv::Mat proj[4];  // index using enum
    cv::Mat projd[4]; 

    // Meta data:
    int dsTime = 1, dsSpace = 1, bitdepth = 0; 

    // Range values:
    double dffminval = 0., dffmaxval = 0., dffrng = 0.;

    void init();
    bool readframe(size_t filenum);
    void accum(const cv::Mat &frame);
    void complete();

    cv::Mat calcDffDouble(const cv::Mat& frame);
    cv::Mat calcDffNative(const cv::Mat& frame);

    // Actual storage:
    std::vector<cv::Mat>* data = new std::vector<cv::Mat>;
    std::vector<cv::Mat>* dataDff = new std::vector<cv::Mat>;

    //cv::Mat rawhistogram;
    //cv::Mat dffhistogram;
};