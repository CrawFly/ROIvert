#pragma once
#include "opencv2/opencv.hpp"
#include "roivertcore.h"

enum class smoothingtype {
    NONE,
    BOX,
    MEDIAN,
    GAUSSIAN,
    BILATERAL
};


class DisplaySettings
{
public:
    DisplaySettings();
    ~DisplaySettings();

    void setContrast(const bool isDff, ROIVert::contrast);
    ROIVert::contrast getContrast(const bool isDff) const noexcept;

    void setProjectionMode(const int projmode) noexcept;
    const int getProjectionMode() const noexcept;

    void setColormap(int cmapint) noexcept;
    const bool useCmap() const noexcept;

    void setSmoothing(ROIVert::smoothing);

    cv::Mat getImage(cv::Mat raw, bool isDff);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();

};