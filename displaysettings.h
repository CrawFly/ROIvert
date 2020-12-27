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
    void setContrast(const bool isDff, const float min, const float max, const float gamma);
    const void getContrast(const bool isDff, float *c);

    void setProjectionMode(const int projmode);
    const int getProjectionMode();

    void setColormap(int cmapint);
    const bool useCmap();

    void setSmoothing(std::tuple<int, int, double, double>);

    cv::Mat getImage(cv::Mat raw, bool isDff);

private:
    bool hasContrast(const bool isDff);
    float contrast[2][3] = { {0,1,1},{0,1,1} }; // [raw,dff][min, max, gamma]

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