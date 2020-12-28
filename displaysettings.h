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
    void setContrast(const bool isDff, ROIVert::contrast);
    ROIVert::contrast getContrast(const bool isDff) const noexcept;

    void setProjectionMode(const int projmode);
    const int getProjectionMode();

    void setColormap(int cmapint);
    const bool useCmap();

    void setSmoothing(ROIVert::smoothing);

    cv::Mat getImage(cv::Mat raw, bool isDff);

private:
    bool hasContrast(const bool isDff);
    ROIVert::contrast Contrast[2] = { {0.,1.,1.}, {0.,1.,1.} };

    void updateLut(const bool isDff);
    cv::Mat lut[2];

    int ProjectionMode = 0; // 0 means no projection

    bool useColormap = false;
    cv::ColormapTypes colormap;

    smoothingtype smoothing = smoothingtype::NONE;
    int smoothsize;
    double smoothsigma;
    double smoothsigmaI;
};