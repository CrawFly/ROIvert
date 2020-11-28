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
    const void getContrast(bool isDff, float *c);

    void setProjectionMode(int projmode);
    const int getProjectionMode();

    void setColormap(int cmapint);
    const bool useCmap();

    void setSmoothing(int smoothType, int smoothSize, double smoothSigma, double smoothSigmaI);

    cv::Mat getImage(cv::Mat raw, bool isDff);

private:
    bool hasContrast(bool isDff);
    float contrast[2][3] = { {0,1,1},{0,1,1} }; // [raw,dff][min, max, gamma]

    void updateLut(bool isDff);
    cv::Mat lut[2];

    int ProjectionMode = 0; // 0 means no projection

    bool useColormap = false;
    cv::ColormapTypes colormap;

    smoothingtype smoothing = smoothingtype::NONE;
    int smoothsize;
    double smoothsigma;
    double smoothsigmaI;
};