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

    VideoData();
    ~VideoData();

    void load(QStringList filelist, int dst, int dss);
    cv::Mat get(bool isDff, int projmode, size_t framenum);
    void getHistogram(bool isDff, std::vector<float>& histogram);
    
    int getWidth();
    int getHeight();
    size_t getNFrames();
    int getdsTime();
    int getdsSpace();
    

    // replace computeTrace with some methods for update/get
    void computeTrace(const cv::Rect cvbb, const cv::Mat mask, const size_t row, cv::Mat& traces);      // will be able to move to private when we have traces in here...

signals:
    void loadProgress(int progress);          // progress goes 0-100

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();


};