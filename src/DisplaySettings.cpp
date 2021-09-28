#include "DisplaySettings.h"

struct DisplaySettings::pimpl
{
    ROIVert::contrast contrast[2] = {{0., 1., 1.}, {0., 1., 1.}};
    cv::Mat lut[2] = {cv::Mat(1, 256, CV_8U), cv::Mat(1, 256, CV_8U)};
    int projectionmode{0}; // 0 means no projection
    bool usecolormap{false};
    cv::ColormapTypes colormap{cv::ColormapTypes::COLORMAP_BONE};
    smoothingtype smoothing = smoothingtype::NONE;
    int smoothsize{0};
    double smoothsigma{0};
    double smoothsigmaI{0};

    void updateLut(const bool isDff);
    bool hasContrast(const bool isDff) const noexcept;
    cv::Mat getImage(cv::Mat raw, bool isDff);
};

DisplaySettings::DisplaySettings() :
    impl(std::make_unique<pimpl>())
{
    impl->updateLut(false);
    impl->updateLut(true);
}
DisplaySettings::~DisplaySettings() = default;
void DisplaySettings::setContrast(const bool isDff, ROIVert::contrast contrast)
{
    impl->contrast[isDff] = contrast;
    impl->updateLut(isDff);
}
ROIVert::contrast DisplaySettings::getContrast(const bool isDff) const noexcept { return impl->contrast[isDff]; }
void DisplaySettings::setProjectionMode(const int projmode) noexcept { impl->projectionmode = projmode; }
const int DisplaySettings::getProjectionMode() const noexcept { return impl->projectionmode; }
void DisplaySettings::setColormap(int cmapint) noexcept
{
    impl->usecolormap = false;
    if (cmapint >= 0)
    {
        impl->usecolormap = true;
        impl->colormap = static_cast<cv::ColormapTypes>(cmapint);
    }
}
const bool DisplaySettings::useCmap() const noexcept { return impl->usecolormap; }
cv::Mat DisplaySettings::getImage(cv::Mat raw, bool isDff) { return impl->getImage(raw, isDff); }
void DisplaySettings::setSmoothing(ROIVert::smoothing s) noexcept
{
    int smoothtype;
    std::tie(smoothtype, impl->smoothsize, impl->smoothsigma, impl->smoothsigmaI) = s;
    impl->smoothing = static_cast<smoothingtype>(smoothtype);
}

// **** pimpl ****
void DisplaySettings::pimpl::updateLut(const bool isDff)
{
    uchar *ptr = lut[isDff].ptr();
    if (ptr == nullptr)
    {
        return;
    }

    const double m = std::get<0>(contrast[isDff]);
    const double rng = std::get<1>(contrast[isDff]) - m;
    const double g = std::get<2>(contrast[isDff]);

    for (int i = 0; i < 256; ++i)
    {
        const double val = pow((i / 255. - m) / rng, g) * 255.;
        ptr[i] = cv::saturate_cast<uchar>(val);
    }
}
bool DisplaySettings::pimpl::hasContrast(const bool isDff) const noexcept
{
    return !(std::get<0>(contrast[isDff]) == 0. &&
             std::get<1>(contrast[isDff]) == 1. &&
             std::get<2>(contrast[isDff]) == 1.);
}
cv::Mat DisplaySettings::pimpl::getImage(cv::Mat raw, bool isDff)
{
    cv::Mat proc(raw.clone());
    const int oddsz = smoothsize + !(smoothsize % 2);

    if (smoothsize > 0)
    {
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
        }
    }

    if (hasContrast(isDff))
    {
        cv::LUT(proc, lut[isDff], proc);
    }

    if (usecolormap)
    {
        cv::Mat res = cv::Mat(proc.size(), CV_8UC3);
        cv::applyColorMap(proc, res, colormap);
        return res;
    }
    else
    {
        return proc;
    }
}
