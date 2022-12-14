#include "VideoData.h"

#include <thread>
#include <QDebug>
#include <QFile>
#include <QRect>
#include <QStringList>

#include "ROIVertEnums.h"


// TODO: Consider making nthreads an option, something that can be set through command line (?) 
//     :: until I've made this decision, don't yank the singlethreaded versions...
// TODO: Consider exposing these for clearer API instead of bools etc.
enum class datatype
{
    RAW = 0,
    DFF = 1
};
enum class datadepth
{
    NATIVE = 0,
    DOUBLE = 1
};

namespace
{
    std::mutex minprojmutex;
    std::mutex maxprojmutex;
    std::mutex sumprojmutex;
    std::mutex histmutex;
    std::mutex cntrmutex;

    void calc_dff_static(const std::vector<cv::Mat> &raw,
                         std::vector<cv::Mat> &dff,
                         const cv::Mat &rawmeanprojd,
                         cv::Mat &dffminproj,
                         cv::Mat &dffmaxproj,
                         cv::Mat &histogram,
                         double dffminval,
                         double dffmaxval,
                         int bitdepth,
                         size_t &ncomplete,
                         std::pair<size_t, size_t> frames = {1, 0})
    {

        if (raw.empty())
            return;

        if (frames.first > frames.second)
        {
            frames = {0, raw.size()};
        }

        auto sz = raw[0].size();
        auto type = raw[0].type();
        const double maxval = pow(2, bitdepth);
        const double dffrng = dffmaxval - dffminval;
        const double alpha = maxval / dffrng;
        const double beta = -1 * maxval * dffminval / dffrng;

        constexpr int chnl = 0;
        constexpr int histsize = 256;
        const float range[] = {0, static_cast<float>(pow(2, bitdepth)) + 1};
        const float *histRange = {range};

        for (size_t frame = frames.first; frame <= frames.second; ++frame)
        {
            cv::Mat thisraw = raw[frame];
            cv::Mat thisdff(sz, CV_64FC1);
            thisraw.convertTo(thisdff, CV_64FC1);
            cv::subtract(thisdff, rawmeanprojd, thisdff);
            cv::divide(thisdff, rawmeanprojd, thisdff);
            cv::Mat thisdff_int(sz, type);
            thisdff.convertTo(thisdff_int, type, alpha, beta);
            dff[frame] = thisdff_int;

            {
                std::lock_guard<std::mutex> guard(minprojmutex);
                dffminproj = cv::min(dffminproj, thisdff_int);
            }
            {
                std::lock_guard<std::mutex> guard(maxprojmutex);
                dffmaxproj = cv::max(dffmaxproj, thisdff_int);
            }
            {
                std::lock_guard<std::mutex> guard(histmutex);
                cv::calcHist(&thisdff_int, 1, &chnl, cv::Mat(), histogram, 1, &histsize, &histRange, true, true);
            }
            {
                std::mutex cntrmutex;
                ncomplete++;
            }
        }
    }

    void calc_raw_static(std::vector<cv::Mat> &data,
                         cv::Mat &minproj,
                         cv::Mat &maxproj,
                         cv::Mat &sumprojd,
                         cv::Mat &histogram,
                         size_t &ncomplete,
                         int bitdepth,
                         std::pair<size_t, size_t> frames = {1, 0})
    {
        if (data.empty())
            return;

        if (frames.first > frames.second)
        {
            frames = {0, data.size()};
        }
        constexpr int chnl = 0;
        constexpr int histsize = 256;
        const float range[] = {0, static_cast<float>(pow(2, bitdepth)) + 1};
        const float *histRange = {range};

        for (size_t frame = frames.first; frame <= frames.second; ++frame)
        {

            cv::Mat &thisdata = data[frame];

            {
                std::lock_guard<std::mutex> guard(minprojmutex);
                minproj = cv::min(minproj, thisdata);
            }
            {
                std::lock_guard<std::mutex> guard(maxprojmutex);
                maxproj = cv::max(maxproj, thisdata);
            }

            {
                std::lock_guard<std::mutex> guard(sumprojmutex);
                cv::accumulate(thisdata, sumprojd);
            }
            {
                std::lock_guard<std::mutex> guard(histmutex);
                cv::calcHist(&thisdata, 1, &chnl, cv::Mat(), histogram, 1, &histsize, &histRange, true, true);
            }
            {
                std::mutex cntrmutex;
                ncomplete++;
            }
        }
    }
}

struct VideoData::pimpl
{
    pimpl(VideoData *parent = nullptr) : par(parent) {
        const auto proccount = std::thread::hardware_concurrency();
        nthreads = std::max(proccount - 1, static_cast<uint>(2));
    }
    void dffNativeToOrig(double &val);
    cv::Mat calcDffDouble(const cv::Mat &frame);
    cv::Mat calcDffNative(const cv::Mat &frame);
    void calcHist(const cv::Mat *frame, cv::Mat &histogram, bool accum);

    int dsTime = 1, dsSpace = 1;
    double dffminval = 0., dffmaxval = 0., dffrng = 0.;
    float framerate;
    cv::Size size;
    size_t nframes{0};
    int mattype{0};
    int bitdepth{8};

    cv::Mat &getProjection(datatype dt, datadepth dd, projection dp)
    {
        auto dti = static_cast<size_t>(dt);
        auto ddi = static_cast<size_t>(dd);
        auto dpi = static_cast<size_t>(dp);
        return projection[dti][ddi][dpi];
    }
    std::vector<cv::Mat> &getData(datatype dt)
    {
        return data[static_cast<size_t>(dt)];
    }
    cv::Mat &getData(datatype dt, size_t frame)
    {
        frame = std::min(frame, nframes - 1); // ? or return blanks ?
        return data[static_cast<size_t>(dt)][frame];
    }
    cv::Mat &getHistogram(datatype dt)
    {
        return histogram[static_cast<size_t>(dt)];
    }
    struct loadinstructions
    {
        loadinstructions(QString fn) : filename(fn) {}
        QString filename;
        bool hasmulti() const { return !sourceframenumber.empty() && sourceframenumber.back() > 0; };
        std::vector<size_t> sourceframenumber;
        std::vector<size_t> destinationframenumber;
    };
    std::vector<loadinstructions> li;
    void inititalize(const std::vector<std::pair<QString, size_t>> &filenameframelist, const int dst, const int dss)
    {
        dsTime = dst;
        dsSpace = dss;

        li.clear();
        li.reserve(filenameframelist.size());
        size_t framecntr = 0;
        size_t cntr = 0;
        for (const auto &f : filenameframelist)
        {
            loadinstructions thisloadinst(f.first);
            for (size_t i = 0; i < f.second; ++i)
            {
                if (cntr++ % dst == 0)
                {
                    thisloadinst.sourceframenumber.push_back(i);
                    thisloadinst.destinationframenumber.push_back(framecntr++);
                }
            }
            if (!thisloadinst.sourceframenumber.empty())
            {
                li.push_back(thisloadinst);
            }
        }
        nframes = framecntr;
    }
    void load()
    {
        emit par->loadProgress(0, 0);
        auto &rawdata = getData(datatype::RAW);
        rawdata.clear();
        rawdata.resize(nframes);
        std::vector<size_t> badframes; // in general the expectation is there are no bad frames, they might occur if tinytiff's header read somehow doesn't match what opencv finds (via tifflib)

        int progcntr = 0;

        for (const auto &l : li) // iterate over load instructions
        {
            int perc = (100 * progcntr++) / (int)li.size();
            emit par->loadProgress(0, perc);

            std::string fn(l.filename.toLocal8Bit().constData());
            if (l.hasmulti())
            {
                std::vector<cv::Mat> incoming;
                cv::imreadmulti(fn, incoming, cv::IMREAD_GRAYSCALE);

                // now move them all into the dataset
                size_t cntr = 0;
                for (auto fr : l.destinationframenumber)
                {
                    if (incoming.size() > cntr)
                    {
                        if (dsSpace > 1)
                        {
                            cv::resize(incoming[cntr], incoming[cntr], incoming[cntr].size() / dsSpace, 0, 0, cv::INTER_NEAREST);
                        }
                        rawdata[fr] = std::move(incoming[cntr]);
                    }
                    else
                    {
                        // shouldn't end up here, this would happen if there was disagreement between header number of frames and what imreadmulti produces
                        badframes.push_back(cntr);
                    }
                    cntr += dsTime;
                }
            }
            else
            {
                rawdata[l.destinationframenumber[0]] = cv::imread(fn, cv::IMREAD_GRAYSCALE);
                if (dsSpace > 1)
                {
                    cv::resize(rawdata[l.destinationframenumber[0]], rawdata[l.destinationframenumber[0]], rawdata[l.destinationframenumber[0]].size() / dsSpace, 0, 0, cv::INTER_NEAREST);
                }
            }
        }
        // clean up bad frames:
        size_t cntr = 0;
        for (const auto &frame : badframes)
        {
            rawdata.erase(rawdata.begin() + frame - cntr++);
        }
        // store final nframes
        nframes = rawdata.size();
        emit par->loadProgress(0, 100);
    }
    void setmetafromraw()
    {
        if (nframes > 0)
        {
            auto fr = getData(datatype::RAW, 0);
            size = fr.size();
            mattype = fr.type();
            bitdepth = fr.depth() == CV_8U ? 8 : 16;
        }
    }
    void accumulateraw()
    {
        // accumulate finds the projections and histogram
        auto &minproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MIN);
        auto &maxproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MAX);
        auto &meanproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MEAN);
        auto &sumproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::SUM);

        auto &minprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MIN);
        auto &maxprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MAX);
        auto &meanprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MEAN);
        auto &sumprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::SUM);

        // loop to get min, max, and sum (stored as double)
        minproj = cv::Mat(size, mattype, pow(2, bitdepth) - 1);
        maxproj = cv::Mat::zeros(size, mattype);
        sumprojd = cv::Mat::zeros(size, CV_64FC1);

        emit par->loadProgress(1, 0);
        int progcntr = 0;
        for (const auto &frame : getData(datatype::RAW))
        {
            int perc = (100 * progcntr++) / (int)nframes;
            emit par->loadProgress(1, perc);
            minproj = cv::min(minproj, frame);
            maxproj = cv::max(maxproj, frame);
            cv::accumulate(frame, sumprojd);
            calcHist(&frame, getHistogram(datatype::RAW), true);
        }
        // now can calculate mean (as double) and then put it into native
        meanprojd = sumprojd / nframes;
        meanprojd.assignTo(meanproj, mattype);

        minproj.assignTo(minprojd, CV_64FC1);
        maxproj.assignTo(maxprojd, CV_64FC1);
        emit par->loadProgress(1, 100);
    }
    void accumulateraw_p()
    {
        auto &minproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MIN);
        auto &maxproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MAX);
        auto &sumprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::SUM);

        minproj = cv::Mat(size, mattype, pow(2, bitdepth) - 1);
        maxproj = cv::Mat::zeros(size, mattype);
        sumprojd = cv::Mat::zeros(size, CV_64FC1);

        size_t threadsize = std::max(std::ceil(nframes / nthreads), 1.);

        size_t ncomplete = 0;
        for (size_t i = 0; i < nframes; i += threadsize)
        {
            size_t nframes_i = i + threadsize > nframes ? nframes - i : threadsize;
            std::pair<size_t, size_t> frames = {i, i + nframes_i - 1};
            std::thread t(calc_raw_static, std::ref(getData(datatype::RAW)),
                          std::ref(getProjection(datatype::RAW, datadepth::NATIVE, projection::MIN)),
                          std::ref(getProjection(datatype::RAW, datadepth::NATIVE, projection::MAX)),
                          std::ref(getProjection(datatype::RAW, datadepth::DOUBLE, projection::SUM)),
                          std::ref(getHistogram(datatype::RAW)),
                          std::ref(ncomplete), bitdepth, frames);

            t.detach();
        }

        while (ncomplete < nframes)
        {
            int perc = (98 * (int)ncomplete) / (int)nframes;
            emit par->loadProgress(1, perc);
        }

        // the last two percent are feeding out to the last couple of projections:

        auto &meanproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MEAN);
        auto &sumproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::SUM);

        auto &minprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MIN);
        auto &maxprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MAX);
        auto &meanprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MEAN);

        meanprojd = sumprojd / nframes;
        emit par->loadProgress(1, 99);
        meanprojd.assignTo(meanproj, mattype);
        minproj.assignTo(minprojd, CV_64FC1);
        maxproj.assignTo(maxprojd, CV_64FC1);
        emit par->loadProgress(1, 100);
    }

    void initializedff()
    {

        auto &dff_minprojd = getProjection(datatype::DFF, datadepth::DOUBLE, projection::MIN);
        auto &dff_maxprojd = getProjection(datatype::DFF, datadepth::DOUBLE, projection::MAX);
        auto &dff_sumprojd = getProjection(datatype::DFF, datadepth::DOUBLE, projection::SUM);
        auto &dff_meanprojd = getProjection(datatype::DFF, datadepth::DOUBLE, projection::MEAN);

        // min and max double can be accomplished by transform. Use these values to define global min/max for depth scaling
        dff_minprojd = calcDffDouble(getProjection(datatype::RAW, datadepth::DOUBLE, projection::MIN));
        dff_maxprojd = calcDffDouble(getProjection(datatype::RAW, datadepth::DOUBLE, projection::MAX));

        cv::minMaxLoc(dff_minprojd, &dffminval, NULL);
        cv::minMaxLoc(dff_maxprojd, NULL, &dffmaxval);
        dffrng = dffmaxval - dffminval;

        // initialize mean as 0s
        dff_meanprojd = cv::Mat::zeros(size, CV_64FC1);
    }

    void accumulatedff()
    {
        auto &dffdata = getData(datatype::DFF);
        dffdata.clear();
        dffdata.reserve(nframes);

        auto &dff_minproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MIN);
        auto &dff_maxproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MAX);
        auto &dff_meanproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MEAN);
        dff_meanproj = cv::Mat::zeros(size, mattype);
        dff_minproj = cv::Mat(size, mattype, pow(2, bitdepth) - 1);
        dff_maxproj = cv::Mat::zeros(size, mattype);
        emit par->loadProgress(2, 0);
        int progcntr = 0;
        for (const auto &frame : getData(datatype::RAW))
        {
            int perc = (100 * progcntr++) / (int)nframes;
            emit par->loadProgress(2, perc);
            auto d = calcDffNative(frame);
            dffdata.push_back(d);
            dff_minproj = cv::min(dff_minproj, d);
            dff_maxproj = cv::max(dff_maxproj, d);
            calcHist(&d, getHistogram(datatype::DFF), true);
        }
        emit par->loadProgress(2, 100);
    }
    void accumulatedff_p()
    {
        auto &dff_minproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MIN);
        auto &dff_maxproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MAX);
        auto &dff_meanproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MEAN);
        dff_meanproj = cv::Mat::zeros(size, mattype);
        dff_minproj = cv::Mat(size, mattype, pow(2, bitdepth) - 1);
        dff_maxproj = cv::Mat::zeros(size, mattype);

        auto &rawdata = getData(datatype::RAW);
        auto &dffdata = getData(datatype::DFF);
        dffdata.clear();
        dffdata.resize(nframes);

        size_t threadsize = std::max(std::ceil(nframes / nthreads), 1.);

        size_t ncomplete = 0;
        for (size_t i = 0; i < nframes; i += threadsize)
        {
            size_t nframes_i = i + threadsize > nframes ? nframes - i : threadsize;
            std::pair<size_t, size_t> frames = {i, i + nframes_i - 1};

            std::thread t(calc_dff_static, std::ref(rawdata), std::ref(dffdata),
                          std::ref(getProjection(datatype::RAW, datadepth::DOUBLE, projection::MEAN)),
                          std::ref(dff_minproj),
                          std::ref(dff_maxproj),
                          std::ref(getHistogram(datatype::DFF)),
                          dffminval, dffmaxval, bitdepth, std::ref(ncomplete), frames);
            t.detach();
        }

        while (ncomplete < nframes)
        {
            int perc = (100 * (int)ncomplete) / (int)nframes;
            emit par->loadProgress(2, perc);
        }
    }

    unsigned int nthreads;
private:
    // projections are 2(raw/dff) x 2(native/double) x 4(projection type)
    std::array<std::array<std::array<cv::Mat, 4>, 2>, 2> projection;
    std::array<std::vector<cv::Mat>, 2> data;
    std::array<cv::Mat, 2> histogram;
    VideoData *par;
};

VideoData::VideoData(QObject *parent) : QObject(parent), impl(std::make_unique<pimpl>(this)) {}
VideoData::~VideoData() = default;
void VideoData::load(std::vector<std::pair<QString, size_t>> filenameframelist, int dst, int dss)
{
    if (filenameframelist.empty())
        return;

    impl->inititalize(filenameframelist, dst, dss);
    impl->load();

    if (impl->nframes == 0)
    {
        return;
    }

    impl->setmetafromraw();
    impl->nthreads > 1 ? impl->accumulateraw_p() : impl->accumulateraw();
    impl->initializedff();
    impl->nthreads > 1 ? impl->accumulatedff_p() : impl->accumulatedff();
}
cv::Mat VideoData::get(bool isDff, int projmode, size_t framenum) const
{
    if (projmode == 0 && framenum >= getNFrames())
    {
        return cv::Mat();
    }

    if (projmode > 0)
    {
        return impl->getProjection(static_cast<datatype>(isDff), datadepth::NATIVE, static_cast<projection>(projmode - 1));
    }

    return impl->getData(static_cast<datatype>(isDff), framenum);
}
void VideoData::getHistogram(bool isDff, std::vector<float> &h) const noexcept
{
    impl->getHistogram(static_cast<datatype>(isDff)).copyTo(h);
}
int VideoData::getWidth() const noexcept { return impl->size.width; }
int VideoData::getHeight() const noexcept { return impl->size.height; }
size_t VideoData::getNFrames() const noexcept { return impl->nframes; }
int VideoData::getdsTime() const noexcept { return impl->dsTime; }
int VideoData::getdsSpace() const noexcept { return impl->dsSpace; }
cv::Mat VideoData::computeTrace(ROIVert::SHAPE s, QRect bb, std::vector<QPoint> pts) const
{
    // Turn the bounding box into a cv box
    const cv::Rect cvbb(static_cast<size_t>(bb.x()),
                        static_cast<size_t>(bb.y()),
                        static_cast<size_t>(bb.width() - 1),
                        static_cast<size_t>(bb.height()) - 1);

    const int w = std::max(bb.width() - 1, 0);
    const int h = std::max(bb.height() - 1, 0);

    const cv::Size sz(w, h);

    cv::Mat mask(sz, CV_8U);
    mask = 0;

    // Get a mask
    switch (s)
    {
    case ROIVert::SHAPE::RECTANGLE:
        mask = 255;
        break;
    case ROIVert::SHAPE::ELLIPSE:
        cv::ellipse(mask, cv::Point(w / 2., h / 2.), cv::Size(w / 2., h / 2.), 0., 0., 360., cv::Scalar(255), cv::FILLED);
        break;
    case ROIVert::SHAPE::SELECT:
        break;
    case ROIVert::SHAPE::POLYGON:
        std::vector<cv::Point> cVertices;
        for (auto &pt : pts)
        {
            cVertices.push_back(cv::Point(pt.x() - bb.left(), pt.y() - bb.top()));
        }
        cv::fillPoly(mask, cVertices, cv::Scalar(255));
        break;
    }

    return computeTrace(cvbb, mask);
}
void VideoData::setFrameRate(float framerate) noexcept
{
    impl->framerate = framerate;
}
float VideoData::getTMax() const noexcept
{
    return impl->nframes / impl->framerate;
}
void VideoData::pimpl::dffNativeToOrig(double &val)
{
    // This helper takes my scaled dff values and translates them back into what they would be in original double space:
    void dffNativeToOrig(float &val);
    double maxval = pow(2, bitdepth); // intmax for this depth
    val = dffminval + dffrng * val / maxval;
}
cv::Mat VideoData::pimpl::calcDffDouble(const cv::Mat &frame)
{
    // This calculates the df/f as a double
    cv::Mat ret(frame.size(), CV_64FC1);
    frame.convertTo(ret, CV_64FC1); // convert frame to double
    auto &mu = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MEAN);

    ret = (ret - mu) / mu;
    return ret;
}
cv::Mat VideoData::pimpl::calcDffNative(const cv::Mat &frame)
{
    // get the double df/f
    cv::Mat dffdbl = calcDffDouble(frame);

    // Convert to uchar, using normalization from range
    const int type = mattype;
    cv::Mat ret(frame.size(), type);
    const double maxval = pow(2, bitdepth);
    const double alpha = maxval / dffrng;
    const double beta = -1 * maxval * dffminval / dffrng;

    dffdbl.convertTo(ret, type, alpha, beta);
    return ret;
}
void VideoData::pimpl::calcHist(const cv::Mat *frame, cv::Mat &histogram, bool accum)
{
    // thin wrapper on opencv calchist
    constexpr int chnl = 0;
    constexpr int histsize = 256;
    const float range[] = {0, static_cast<float>(pow(2, bitdepth)) + 1}; // the upper boundary is exclusive
    const float *histRange = {range};

    cv::calcHist(frame, 1, &chnl, cv::Mat(), histogram, 1, &histsize, &histRange, true, accum);
}
cv::Mat VideoData::computeTrace(const cv::Rect cvbb, const cv::Mat mask) const
{
    auto res = cv::Mat(1, static_cast<int>(getNFrames()), CV_32FC1);
    if (cvbb.width <= 0 || cvbb.height <= 0)
    {
        res = 0;
        return res;
    }

    cv::Mat boundedMu = get(false, 3, 0)(cvbb);
    cv::bitwise_and(mask, boundedMu > 0, mask);

    for (int i = 0; i < getNFrames(); ++i)
    {
        cv::Mat boundedRaw = get(false, 0, i)(cvbb);
        boundedRaw.convertTo(boundedRaw, CV_32FC1);
        boundedMu.convertTo(boundedMu, CV_32FC1);
        cv::Mat boundedDff = (boundedRaw - boundedMu) / boundedMu;
        double mu = cv::mean(boundedDff, mask)[0];

        res.at<float>(0, i) = mu;
    }

    return res;
}


void VideoData::setNthreads(unsigned int nthreads) {
    impl->nthreads = nthreads;
}

unsigned int VideoData::getNthreads() const noexcept {
    return impl->nthreads;
}
