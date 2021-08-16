#include "displaysettings.h"

DisplaySettings::DisplaySettings(){
    lut[0] = cv::Mat(1, 256, CV_8U);
    lut[1] = cv::Mat(1, 256, CV_8U);
    updateLut(false);
    updateLut(true);
}

void DisplaySettings::setContrast(const bool isDff, ROIVert::contrast c) {
    Contrast[isDff] = c;
    updateLut(isDff);
}
ROIVert::contrast DisplaySettings::getContrast(const bool isDff) const noexcept {
    return Contrast[isDff];
}

void DisplaySettings::updateLut(const bool isDff) {
    uchar* ptr = lut[isDff].ptr();
    
    double m = std::get<0>(Contrast[isDff]);
    double rng = std::get<1>(Contrast[isDff]) - m;
    double g = std::get<2>(Contrast[isDff]);

    for (int i = 0; i < 256; ++i) {
        double val = pow((i / 255. - m) / rng, g) * 255.;
        ptr[i] = cv::saturate_cast<uchar>(val);
    }
}
bool DisplaySettings::hasContrast(const bool isDff) {
    return !(std::get<0>(Contrast[isDff]) == 0. &&
        std::get<1>(Contrast[isDff]) == 1. &&
        std::get<2>(Contrast[isDff]) == 1.);
}
void DisplaySettings::setProjectionMode(const int projmode) {
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
        colormap = static_cast<cv::ColormapTypes>(cmapint);
    }
}

const bool DisplaySettings::useCmap() { return useColormap; }

cv::Mat DisplaySettings::getImage(cv::Mat raw, bool isDff){
    cv::Mat proc(raw.clone());
    const int oddsz = smoothsize + !(smoothsize % 2);

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

    if (hasContrast(isDff)){
        cv::LUT(proc, lut[isDff], proc);
    }
    
    if (useCmap()) {
        cv::Mat res = cv::Mat(proc.size(), CV_8UC3);
        cv::applyColorMap(proc, res, colormap);
        return res;
    }
    else {
        return proc;
    }
}
void DisplaySettings::setSmoothing(ROIVert::smoothing s) {
    int smoothtype;
    std::tie(smoothtype, smoothsize, smoothsigma, smoothsigmaI) = s;
    smoothing = static_cast<smoothingtype>(smoothtype);
}