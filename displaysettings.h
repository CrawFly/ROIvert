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
    void setContrast(const bool isDff, contrast);
    contrast getContrast(const bool isDff) const noexcept;

    void setProjectionMode(const int projmode);
    const int getProjectionMode();

    void setColormap(int cmapint);
    const bool useCmap();

    void setSmoothing(std::tuple<int, int, double, double>);

    cv::Mat getImage(cv::Mat raw, bool isDff);

private:
    bool hasContrast(const bool isDff);
    contrast Contrast[2] = { std::make_tuple(0.,1.,1.),std::make_tuple(0.,1.,1.) }; 

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