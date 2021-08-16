#pragma once

#include <QObject>
#include "opencv2/opencv.hpp"
class QStringList;
namespace ROIVert {
    enum class SHAPE;
}


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
    cv::Mat get(bool isDff, int projmode, size_t framenum) const;
    void getHistogram(bool isDff, std::vector<float>& histogram) const noexcept;
    
    int getWidth() const noexcept;
    int getHeight() const noexcept;
    size_t getNFrames() const noexcept;
    int getdsTime() const noexcept;
    int getdsSpace() const noexcept;
    
    float getTMax() const noexcept;
    void setFrameRate(float framerate) noexcept;

    // TODO: KILL this OLD COMPUTERS!
    //void computeTrace(const cv::Rect cvbb, const cv::Mat mask, const size_t row, cv::Mat& traces);      // will be able to move to private when we have traces in here...

    // todo: this one could move to private, idc.
    cv::Mat computeTrace(const cv::Rect cvbb, const cv::Mat mask) const;
    cv::Mat computeTrace(ROIVert::SHAPE, QRect, std::vector<QPoint>) const;


signals:
    void loadProgress(int progress);          // progress goes 0-100

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};