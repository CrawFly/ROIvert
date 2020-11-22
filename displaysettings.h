#pragma once
#include "opencv2/opencv.hpp"

class DisplaySettings
{
public:
    DisplaySettings();
    void setContrast(bool isDff, float min, float max, float gamma);
    void getRawContrast(bool isDff, float *c);
    cv::Mat getLut(bool isDff);
    bool hasContrast(bool isDff);

private:
    void updateLut(bool isDff);
    float contrast[2][3] = { {0,1,1},{0,1,1} }; // [raw,dff][min, max, gamma]
    cv::Mat lut[2];

};