#include "VideoData.h"

#include <QDebug>
#include <QFile>
#include <QRect>
#include <QStringList>

#include "ROIVertEnums.h"

// TODO: Consider exposing these for clearer API instead of bools etc.
enum class datatype {
    RAW = 0,
    DFF = 1
};
enum class datadepth {
    NATIVE = 0,
    DOUBLE = 1
};

struct VideoData::pimpl
{
    void init();
    void accum(const cv::Mat& frame, bool isDff);
    void complete();

    void dffNativeToOrig(double& val);
    cv::Mat calcDffDouble(const cv::Mat& frame);
    cv::Mat calcDffNative(const cv::Mat& frame);

    void readframe(size_t filenum);
    void readmulti(const QString& filename, VideoData* par);
    void calcHist(const cv::Mat* frame, cv::Mat& histogram, bool accum);

    QStringList files;
    //int width = 0, height = 0, nframes = 0, mattype = 0;
    cv::Mat proj[2][4]; // outer is raw|dff; inner is indexed by enum
    cv::Mat projdbl[2][4];
    int dsTime = 1, dsSpace = 1;
    double dffminval = 0., dffmaxval = 0., dffrng = 0.;
    cv::Mat histogram[2]; // [raw|dff]
    std::vector<cv::Mat> data[2];
    float framerate;


    cv::Size size;
    size_t nframes{ 0 };
    int mattype{ 0 };
    int bitdepth{ 8 };

    // ** new stuff starts here
    cv::Mat& getProjection(datatype dt, datadepth dd, projection dp) {
        auto dti = static_cast<size_t>(dt);
        auto ddi = static_cast<size_t>(dd);
        auto dpi = static_cast<size_t>(dp);
        return projection[dti][ddi][dpi];
    }

    // projections are 2(raw/dff) x 2(native/double) x 4(projection type)
    std::array<std::array<std::array<cv::Mat, 4>, 2>, 2> projection;

    std::vector<cv::Mat> rawdata;
    cv::Mat rawhistogram;
    
    std::vector<cv::Mat> dffdata;
    cv::Mat dffhistogram;

    
    // a simple strategy for choosing correct frame numbers: 
    struct loadinstructions {
        loadinstructions(QString fn) : filename(fn) {}
        QString filename;
        bool hasmulti() const { return !sourceframenumber.empty() && sourceframenumber.back() > 0; };
        std::vector<size_t> sourceframenumber;
        std::vector<size_t> destinationframenumber;
    };
    std::vector<loadinstructions> li;

    void inititalize(const std::vector<std::pair<QString, size_t>>& filenameframelist, const int dst, const int dss) {
        dsTime = dst;
        dsSpace = dss;

        li.clear();
        li.reserve(filenameframelist.size());
        size_t framecntr = 0;
        size_t cntr = 0;
        for (const auto& f : filenameframelist) 
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
            if (!thisloadinst.sourceframenumber.empty()) {
                li.push_back(thisloadinst);
            }
        }
        nframes = framecntr;
    }
    void load() {
        rawdata.clear();
        rawdata.resize(nframes);
        std::vector<size_t> badframes; // in general the expectation is there are no bad frames, they might occur if tinytiff's header read somehow doesn't match what opencv finds (via tifflib)
        
        for (const auto& l : li) // iterate over load instructions
        { 
            std::string fn(l.filename.toLocal8Bit().constData());
            if (l.hasmulti()) 
            {
                std::vector<cv::Mat> incoming;
                cv::imreadmulti(fn, incoming, cv::IMREAD_GRAYSCALE);
            
                // now move them all into the dataset
                size_t cntr = 0;
                for (auto fr : l.destinationframenumber) {
                    if (incoming.size() > cntr) 
                    {
                        if (dsSpace > 1) 
                        {
                            cv::resize(incoming[cntr], incoming[cntr], incoming[cntr].size() / dsSpace, 0, 0, cv::INTER_NEAREST);
                        }
                        rawdata[fr] = std::move(incoming[cntr++]);
                    }
                    else 
                    {
                        // shouldn't end up here, this would happen if there was disagreement between header number of frames and what imreadmulti produces
                        badframes.push_back(cntr++);
                    }
                }
            }
            else {
                rawdata[l.destinationframenumber[0]] = cv::imread(fn, cv::IMREAD_GRAYSCALE);
                if (dsSpace > 1) 
                {
                    cv::resize(rawdata[l.destinationframenumber[0]], rawdata[l.destinationframenumber[0]], rawdata[l.destinationframenumber[0]].size() / dsSpace, 0, 0, cv::INTER_NEAREST);
                }
            }
        }
        // clean up bad frames:
        size_t cntr = 0;
        for (const auto& frame:badframes) {
            rawdata.erase(rawdata.begin() + frame - cntr++);
        }
        // store final nframes
        nframes = rawdata.size();
    }
    void setmetafromraw() {
        if (nframes > 0) 
        {
            size = rawdata[0].size();
            mattype = rawdata[0].type();
            bitdepth = rawdata[0].depth() == CV_8U ? 8 : 16;
        }
    }

    void accumulateraw() {
        // accumulate finds the projections and histogram
        auto& minproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MIN);
        auto& maxproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MAX);
        auto& meanproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::MEAN);
        auto& sumproj = getProjection(datatype::RAW, datadepth::NATIVE, projection::SUM);
    
        auto& minprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MIN);
        auto& maxprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MAX);
        auto& meanprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MEAN);
        auto& sumprojd = getProjection(datatype::RAW, datadepth::DOUBLE, projection::SUM);
        
        // loop to get min, max, and sum (stored as double)
        minproj = cv::Mat(size, mattype, pow(2, bitdepth) - 1);
        maxproj = cv::Mat::zeros(size, mattype);
        sumprojd = cv::Mat::zeros(size, CV_64FC1);
        for (const auto& frame : rawdata) 
        {
            minproj = cv::min(minproj, frame);
            maxproj = cv::max(maxproj, frame);
            cv::accumulate(frame, sumprojd);
            calcHist(&frame, rawhistogram, true);
        }
        // now can calculate mean (as double) and then put it into native
        meanprojd = sumprojd / nframes;
        meanprojd.assignTo(meanproj, mattype);
        
        minproj.assignTo(minprojd, CV_64FC1);
        maxproj.assignTo(maxprojd, CV_64FC1);
    }
    void initializedff() {
        
        auto& dff_minprojd = getProjection(datatype::DFF, datadepth::DOUBLE, projection::MIN);
        auto& dff_maxprojd = getProjection(datatype::DFF, datadepth::DOUBLE, projection::MAX);
        auto& dff_sumprojd = getProjection(datatype::DFF, datadepth::DOUBLE, projection::SUM);
        auto& dff_meanprojd = getProjection(datatype::DFF, datadepth::DOUBLE, projection::MEAN);
    
        // min and max double can be accomplished by transform. Use these values to define global min/max for depth scaling
        dff_minprojd = calcDffDouble(getProjection(datatype::RAW, datadepth::DOUBLE, projection::MIN));
        dff_maxprojd = calcDffDouble(getProjection(datatype::RAW, datadepth::DOUBLE, projection::MAX));;
        cv::minMaxLoc(dff_minprojd, &dffminval, NULL);
        cv::minMaxLoc(dff_maxprojd, NULL, &dffmaxval);
        dffrng = dffmaxval - dffminval;
        
        // initialize mean as 0s
        dff_meanprojd = cv::Mat::zeros(size, CV_64FC1);

    }
    void calculatedff() {
        dffdata.clear();
        dffdata.reserve(nframes);
        
        auto& dff_minproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MIN);
        auto& dff_maxproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MAX);
        auto& dff_meanproj = getProjection(datatype::DFF, datadepth::NATIVE, projection::MEAN);
        
        dff_meanproj = cv::Mat::zeros(size, mattype);
        dff_minproj = cv::Mat(size, mattype, pow(2, bitdepth) - 1);
        dff_maxproj = cv::Mat::zeros(size, mattype);
        for (const auto& frame : rawdata) 
        {
            auto d = calcDffNative(frame);
            dffdata.push_back(d);
            dff_minproj = cv::min(dff_minproj, d);
            dff_maxproj = cv::max(dff_maxproj, d);
            calcHist(&d, dffhistogram, true);
        }
    }
};

VideoData::VideoData(QObject* parent) : QObject(parent), impl(std::make_unique<pimpl>()) {}
VideoData::~VideoData() = default;

void VideoData::load(QStringList filelist, int dst, int dss, bool isfolder)
{
    if (filelist.empty())
    {
        return;
    } // should clear? This is just failsafe so i think i'm okay

    impl->files.clear();
    impl->dsTime = dst;
    impl->dsSpace = dss;
    impl->files.reserve(filelist.size() / dst);
    for (int i = 0; i < filelist.size(); i += dst)
    {
        impl->files.push_back(filelist[i]);
    }
    impl->init();

    if (isfolder)
    {
        for (size_t i = 0; i < getNFrames(); ++i)
        {
            impl->readframe(i);
            impl->accum(impl->data[0][i], false);
            emit loadProgress((100 - (50)) * static_cast<float>(i) / getNFrames());
        }
    }
    else
    {
        impl->readmulti(filelist[0], this);
        emit loadProgress(50);
    }

    impl->complete();

    for (size_t i = 0; i < getNFrames(); i++)
    {
        impl->data[1][i] = impl->calcDffNative(impl->data[0][i]);
        impl->accum(impl->data[1][i], true);
        emit loadProgress(50 + 50 * (float)i / getNFrames());
    }
}

namespace {
    void print_mat(const cv::Mat& mat) {
        auto w = mat.size().width;
        auto h = mat.size().height;
        

        for (size_t i = 0; i < h; ++i) {
            QString str;
            for (size_t j = 0; j < w; ++j) {
                if(mat.type() == CV_8U)
                    str.push_back(QString::number(mat.at<uint8_t>(i, j))+",");
                else
                    str.push_back(QString::number(mat.at<double>(i, j))+",");
            }
        }
        
    }
}

void VideoData::load(std::vector<std::pair<QString, size_t>> filenameframelist, int dst, int dss) {
    if (filenameframelist.empty())
        return;

    impl->inititalize(filenameframelist, dst, dss);
    impl->load();

    if (impl->nframes == 0) {
        return;
    }

    impl->setmetafromraw();
    impl->accumulateraw();
    impl->initializedff();
    impl->calculatedff();
}

cv::Mat VideoData::get(bool isDff, int projmode, size_t framenum) const
{
    if (projmode == 0 && framenum >= getNFrames())
    {
        return cv::Mat();
    }

    // cast the bool into the enum...
    
    if (projmode > 0) {
        return impl->getProjection(static_cast<datatype>(isDff), datadepth::NATIVE, static_cast<projection>(projmode - 1));
    }


    if (!isDff) 
        return impl->rawdata[framenum];
    return impl->dffdata[framenum];
}

void VideoData::getHistogram(bool isDff, std::vector<float> &h) const noexcept
{
    impl->histogram[isDff].copyTo(h);
}
int VideoData::getWidth() const noexcept { return impl->size.width; }
int VideoData::getHeight() const noexcept { return impl->size.height; }
size_t VideoData::getNFrames() const noexcept { return impl->nframes; }
int VideoData::getdsTime() const noexcept { return impl->dsTime; }
int VideoData::getdsSpace() const noexcept { return impl->dsSpace; }

void VideoData::pimpl::init()
{
    data[0].clear();
    data[0].resize(files.size());
    data[1].clear();
    data[1].resize(files.size());
    if (files.empty())
    {
        return;
    };

    cv::Mat first = cv::imread(files[0].toLocal8Bit().constData(), cv::IMREAD_GRAYSCALE);
    if (first.depth() == CV_8U)
    {
        bitdepth = 8;
    }
    else if (first.depth() == CV_16U)
    {
        bitdepth = 16;
    }
    else
    {
        qFatal("Bad file type");
    }
    if (dsSpace > 1)
    {
        cv::resize(first, first, cv::Size(), 1. / dsSpace, 1. / dsSpace, cv::INTER_NEAREST);
    }
    auto width = first.size().width;
    auto height = first.size().height;
    nframes = files.size();
    mattype = first.type();

    // initialize projections
    for (int i = 0; i < 2; i++)
    {
        proj[i][(int)projection::MIN] = cv::Mat(first.size(), first.type(), 255);
        proj[i][(int)projection::MAX] = cv::Mat::zeros(first.size(), mattype);
        proj[i][(int)projection::MEAN] = cv::Mat::zeros(first.size(), mattype);
        proj[i][(int)projection::SUM] = cv::Mat::zeros(first.size(), mattype);
    }

    // now cast proj to doubles to initialize:
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            proj[i][j].convertTo(projdbl[i][j], CV_64FC1);
        }
    }

    // initialize histograms
    histogram[0] = cv::Mat::zeros(1, 256, CV_32F);
    histogram[1] = cv::Mat::zeros(1, 256, CV_32F);
}
void VideoData::pimpl::accum(const cv::Mat & frame, bool isDff)
{
    // accumulate (raw) min, max, histogram
    proj[isDff][0] = cv::min(proj[isDff][0], frame);
    proj[isDff][1] = cv::max(proj[isDff][1], frame);

    cv::accumulate(frame, projdbl[isDff][3]);
    calcHist(&frame, histogram[isDff], true);
}
void VideoData::pimpl::complete()
{
    projdbl[0][static_cast<size_t>(VideoData::projection::MEAN)] = projdbl[0][static_cast<size_t>(VideoData::projection::SUM)] / nframes;
    projdbl[0][static_cast<size_t>(VideoData::projection::MEAN)].assignTo(proj[0][static_cast<size_t>(VideoData::projection::MEAN)], mattype);
    proj[0][static_cast<size_t>(VideoData::projection::MIN)].assignTo(projdbl[0][static_cast<size_t>(VideoData::projection::MIN)], CV_64FC1);
    proj[0][static_cast<size_t>(VideoData::projection::MAX)].assignTo(projdbl[0][static_cast<size_t>(VideoData::projection::MAX)], CV_64FC1);

    cv::Mat mindff = calcDffDouble(projdbl[0][static_cast<size_t>(VideoData::projection::MIN)]);
    cv::Mat maxdff = calcDffDouble(projdbl[0][static_cast<size_t>(VideoData::projection::MAX)]);
    cv::minMaxLoc(mindff, &dffminval, NULL);
    cv::minMaxLoc(maxdff, NULL, &dffmaxval);
    dffrng = dffmaxval - dffminval;
}

void VideoData::pimpl::dffNativeToOrig(double& val)
{
    // This helper takes my scaled dff values and translates them back into what they would be in original double space:
    void dffNativeToOrig(float& val);
    double maxval = pow(2, bitdepth); // intmax for this depth
    val = dffminval + dffrng * val / maxval;
}
cv::Mat VideoData::pimpl::calcDffDouble(const cv::Mat& frame)
{
    // This calculates the df/f as a double
    cv::Mat ret(frame.size(), CV_64FC1);
    frame.convertTo(ret, CV_64FC1); // convert frame to double

    auto& mu = getProjection(datatype::RAW, datadepth::DOUBLE, projection::MEAN);
    cv::subtract(ret, mu, ret);
    cv::divide(ret, mu, ret);
    return ret;
}
cv::Mat VideoData::pimpl::calcDffNative(const cv::Mat & frame)
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

void VideoData::pimpl::readframe(size_t ind)
{
    data[0][ind] = cv::Mat(size.height, size.width, mattype);

    std::string filename = files[ind].toLocal8Bit().constData();
    cv::Mat image = cv::imread(filename, cv::IMREAD_GRAYSCALE);

    if (dsSpace > 1)
    {
        cv::resize(image, data[0][ind], data[0][ind].size(), 0, 0, cv::INTER_NEAREST);
        return;
    }

    data[0][ind] = image;
}
void VideoData::pimpl::readmulti(const QString & filename, VideoData * par)
{
    std::string fn = filename.toLocal8Bit().constData();
    std::vector<cv::Mat> tallstack;

    emit par->loadProgress(1.f);

    cv::imreadmulti(fn, tallstack, cv::IMREAD_GRAYSCALE);

    {
        int cnt = 0;
        int mod = dsTime;
        data[0].resize(tallstack.size() / dsTime);
        std::copy_if(tallstack.begin(), tallstack.end(), data[0].begin(), [&cnt, &mod](cv::Mat fr) -> bool
        { return ++cnt % mod == 0; });
    }

    float cnt = 0;
    for (auto& image : data[0])
    {
        if (dsSpace > 1)
        {
            cv::resize(image, image, image.size() / dsSpace, 0, 0, cv::INTER_NEAREST);
        }
        accum(image, false);
        emit par->loadProgress(50.f * (++cnt / data[0].size()));
    }

    nframes = data[0].size();
    data[1].resize(nframes);
}
void VideoData::pimpl::calcHist(const cv::Mat * frame, cv::Mat & histogram, bool accum)
{
    // thin wrapper on opencv calchist
    constexpr int chnl = 0;
    constexpr int histsize = 256;
    const float range[] = { 0, static_cast<float>(pow(2, bitdepth)) + 1 }; //the upper boundary is exclusive
    const float* histRange = { range };

    cv::calcHist(frame, 1, &chnl, cv::Mat(), histogram, 1, &histsize, &histRange, true, accum);
}
cv::Mat VideoData::computeTrace(const cv::Rect cvbb, const cv::Mat mask) const
{
    auto res = cv::Mat(1, getNFrames(), CV_32FC1);
    if (cvbb.width <= 0 || cvbb.height <= 0)
    {
        res = 0;
        return res;
    }

    cv::Mat boundedMu = get(false, 3, 0)(cvbb);
    cv::bitwise_and(mask, boundedMu > 0, mask);

    for (size_t i = 0; i < getNFrames(); ++i)
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
        for (auto& pt : pts)
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