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
const void DisplaySettings::getContrast(bool isDff, float *c) {
    c[0] = contrast[isDff][0];
    c[1] = contrast[isDff][1];
    c[2] = contrast[isDff][2];
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
const int DisplaySettings::getProjectionMode() {
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

const bool DisplaySettings::useCmap() { return useColormap; }

cv::Mat DisplaySettings::getImage(cv::Mat raw, bool isDff){
    cv::Mat proc(raw.clone());
    int oddsz = smoothsize + (!(bool)(smoothsize % 2));

    if (smoothsize > 0) {
        switch (smoothing)
        {
        case smoothingtype::NONE:
            break;
        case smoothingtype::BOX:
            cv::blur(proc, proc, cv::Size(smoothsize, smoothsize));
            break;
        case smoothingtype::MEDIAN:
            cv::medianBlur(proc, proc, oddsz);
            break;
        case smoothingtype::GAUSSIAN:
            cv::GaussianBlur(proc, proc, cv::Size(oddsz, oddsz), smoothsigma, smoothsigma);
            break;
        case smoothingtype::BILATERAL:
        {
            cv::Mat bilat(raw.size(), raw.type());
            cv::bilateralFilter(proc, bilat, smoothsize, smoothsigmaI, smoothsigma);
            proc = bilat.clone();
        }
        break;
        default:
            break;
        }
    }

    if (hasContrast(isDff)){cv::LUT(proc, lut[isDff], proc);}
    
    if (useCmap()) {
        cv::Mat res = cv::Mat(proc.size(), CV_8UC3);
        cv::applyColorMap(proc, res, colormap);
        return res;
    }
    else {
        return proc;
    }
}
void DisplaySettings::setSmoothing(int type, int sz, double sig, double sig_i) {
    smoothing = (smoothingtype)type;
    smoothsize = sz;
    smoothsigma = sig;
    smoothsigmaI = sig_i;
}