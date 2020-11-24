#include "displaysettings.h"

DisplaySettings::DisplaySettings(){
    lut[0] = cv::Mat(1, 256, CV_8U);
    lut[1] = cv::Mat(1, 256, CV_8U);
}

void DisplaySettings::setContrast(bool isDff, float min, float max, float gamma) {
    contrast[isDff][0] = min;
    contrast[isDff][1] = max;
    contrast[isDff][2] = gamma;
    updateLut(isDff);
}
void DisplaySettings::getRawContrast(bool isDff, float *c) {
    c[0] = contrast[isDff][0];
    c[1] = contrast[isDff][1];
    c[2] = contrast[isDff][2];
}
cv::Mat DisplaySettings::getLut(bool isDff) {
    return lut[isDff];
}
void DisplaySettings::updateLut(bool isDff) {
    uchar* ptr = lut[isDff].ptr();
    
    double m = contrast[isDff][0];
    double rng = contrast[isDff][1] - m;
    double g = contrast[isDff][2];

    for (int i = 0; i < 256; ++i) {
        double val = pow((i / 255. - m) / rng, g) * 255.;
        ptr[i] = cv::saturate_cast<uchar>(val);
    }
}
bool DisplaySettings::hasContrast(bool isDff) {
    return contrast[isDff][0] != 0. || contrast[isDff][1] != 1. || contrast[isDff][2] != 1;
}
void DisplaySettings::setProjectionMode(int projmode) {
    ProjectionMode = projmode;
}
int DisplaySettings::getProjectionMode() {
    return ProjectionMode;
}

void DisplaySettings::setColormap(int cmapint) {
    if (cmapint < 0){
        useColormap = false;
    }
    else {
        useColormap = true;
        colormap = (cv::ColormapTypes)cmapint;
    }
}

cv::ColormapTypes DisplaySettings::getColormap() { return colormap; }
bool DisplaySettings::useCmap() { return useColormap; }