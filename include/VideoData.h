#pragma once

#include <QObject>
#include "opencv2/opencv.hpp"
class QStringList;
namespace ROIVert
{
    enum class SHAPE;
}

class VideoData : public QObject
{
    Q_OBJECT

public:
    VideoData(QObject *parent = nullptr);
    ~VideoData();

    enum class projection
    {
        MIN,
        MAX,
        MEAN,
        SUM
    };
    void load(std::vector<std::pair<QString, size_t>> filenameframelist, int dst, int dss);

    cv::Mat get(bool isDff, int projmode, size_t framenum) const;
    void getHistogram(bool isDff, std::vector<float> &histogram) const noexcept;

    int getWidth() const noexcept;
    int getHeight() const noexcept;
    size_t getNFrames() const noexcept;
    int getdsTime() const noexcept;
    int getdsSpace() const noexcept;

    float getTMax() const noexcept;
    void setFrameRate(float framerate) noexcept;

    cv::Mat computeTrace(const cv::Rect cvbb, const cv::Mat mask) const;
    cv::Mat computeTrace(ROIVert::SHAPE, QRect, std::vector<QPoint>) const;

signals:
    void loadProgress(const int level, const int progress);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
