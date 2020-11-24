#pragma once
#include "opencv2/opencv.hpp"

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
    void setContrast(bool isDff, float min, float max, float gamma);
    void getRawContrast(bool isDff, float *c);

    // all bunch of these could move to private now?
    void setProjectionMode(int projmode);
    int getProjectionMode();
    void setColormap(int cmapint);
    cv::ColormapTypes getColormap();
    // ^^

    bool useCmap();
    cv::Mat getImage(cv::Mat raw, bool isDff);

private:
    bool hasContrast(bool isDff);
    float contrast[2][3] = { {0,1,1},{0,1,1} }; // [raw,dff][min, max, gamma]

    void updateLut(bool isDff);
    cv::Mat getLut(bool isDff);
    cv::Mat lut[2];

    int ProjectionMode = 0; // 0 means no projection

    bool useColormap = false;
    cv::ColormapTypes colormap;

    smoothingtype smoothing = smoothingtype::NONE;
    int smoothsize;
    double smoothsigma;
    double smoothsigmaI;
};