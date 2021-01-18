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
    cv::Mat getFrame(bool isDff, size_t frameindex);
    cv::Mat getProjection(bool isDff, VideoData::projection proj);

    // this is maybe temporary until i refactor into a nice clean array
    cv::Mat get(bool isDff, int projmode, size_t framenum);
    
    void setStoreDff(bool enabled);
    bool getStoreDff();
    
    // these get the total histograms
    void getHistogram(bool isDff, std::vector<float>& histogram);
    void getHistogram(bool isDff, std::vector<float>& histogram, size_t framenum);
    
    int getWidth();
    int getHeight();
    size_t getNFrames();
    int getdsTime();
    int getdsSpace();

    void dffNativeToOrig(double &val);
    std::vector<double> calcTrace(cv::Rect cvbb, cv::Mat mask);
    void computeTrace(const cv::Rect cvbb, const cv::Mat mask, const size_t row, cv::Mat& traces);

signals:
    void loadProgress(int progress);          // progress goes 0-100

private:
    bool storeDff = true; // For now many things sort of depnd on this being true, but there's some flexibility here
    QStringList files;
    int width=0, height=0, nframes=0;

    // Projections:
    cv::Mat proj[2][4];  // outer is raw|dff; inner is indexed by enum
    cv::Mat projdbl[2][4];

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

    // Histogram stuff:
    void calcHist(const cv::Mat* frame, cv::Mat& histogram, bool accum);

    cv::Mat histogram[2]; // [raw|dff]

    // Actual storage:
    std::vector<cv::Mat>* data[2] = { new std::vector<cv::Mat>, new std::vector<cv::Mat> };
};