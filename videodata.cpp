#include "videodata.h"
#include "qdebug.h"

#include <QFile>
#include <QStringList>
#include <QRect>
#include "ROIVertEnums.h"






struct VideoData::pimpl {

    void init();
    void accum(const cv::Mat& frame, bool isDff);
    void complete();

    void dffNativeToOrig(double& val);
    cv::Mat calcDffDouble(const cv::Mat& frame);
    cv::Mat calcDffNative(const cv::Mat& frame);

    void readframe(size_t filenum);
    void calcHist(const cv::Mat* frame, cv::Mat& histogram, bool accum);

    QStringList files;
    int width = 0, height = 0, nframes = 0, mattype=0;
    cv::Mat proj[2][4];  // outer is raw|dff; inner is indexed by enum
    cv::Mat projdbl[2][4];
    int dsTime = 1, dsSpace = 1, bitdepth = 0;
    double dffminval = 0., dffmaxval = 0., dffrng = 0.;
    cv::Mat histogram[2]; // [raw|dff]
    std::vector<cv::Mat> data[2]; 

};


VideoData::VideoData() = default;
VideoData::~VideoData() = default;


void VideoData::load(QStringList filelist, int dst, int dss){
    if (filelist.empty()) { return; } // should clear? This is just failsafe so i think i'm okay
    impl->files.clear();
    impl->files.reserve(filelist.size() / dst);
    for (int i = 0; i < filelist.size(); i+=dst) {
        impl->files.push_back(filelist[i]);
    }
    impl->dsTime = dst;
    impl->dsSpace = dss;

    impl->init();
    for (size_t i = 0; i < getNFrames(); ++i) {
        impl->readframe(i);
        impl->accum(impl->data[0][i],false);
        emit loadProgress((100 - (50)) * static_cast<float>(i) / getNFrames());
    }
   
    impl->complete();

    for (size_t i = 0; i < getNFrames(); i++) {
        impl->data[1][i]=impl->calcDffNative(impl->data[0][i]);
        impl->accum(impl->data[1][i], true);
        emit loadProgress(50 + 50 * (float)i / getNFrames());
    }
}


cv::Mat VideoData::get(bool isDff, int projmode, size_t framenum) const {
    if (projmode > 0) {
        return impl->proj[isDff][(size_t)static_cast<VideoData::projection>(projmode - 1)];
    }
    if (framenum >= getNFrames()) {
        return cv::Mat();
    }
    return impl->data[isDff][framenum];
}

void VideoData::getHistogram(bool isDff, std::vector<float>& h) const noexcept { impl->histogram[isDff].copyTo(h); }
int VideoData::getWidth() const noexcept { return impl->width; }
int VideoData::getHeight() const noexcept { return impl->height; }
size_t VideoData::getNFrames() const noexcept { return impl->nframes; }
int VideoData::getdsTime() const noexcept { return impl->dsTime; }
int VideoData::getdsSpace() const noexcept { return impl->dsSpace; }


void VideoData::pimpl::init() {
    data[0].clear();
    data[0].resize(files.size());
    data[1].clear();
    data[1].resize(files.size());
    if (files.empty()) { return; };


    cv::Mat first = cv::imread(files[0].toLocal8Bit().constData(), cv::IMREAD_GRAYSCALE);
    if (first.depth() == CV_8U) { bitdepth = 8; }
    else if (first.depth() == CV_16U) { bitdepth = 16; }
    else { qFatal("Bad file type"); }
    if (dsSpace > 1) {
        cv::resize(first, first, cv::Size(), 1. / dsSpace, 1. / dsSpace, cv::INTER_NEAREST);
    }
    width = first.size().width;
    height = first.size().height;
    nframes = files.size();
    mattype = first.type();

    // initialize projections
    for (int i = 0; i < 2; i++) { // looping over raw and dff
        proj[i][(int)projection::MIN] = first.clone(); // initialize min and max as the first frame, will compare on each load
        proj[i][(int)projection::MAX] = first.clone();
        proj[i][(int)projection::MEAN] = cv::Mat::zeros(first.size(), mattype);
        proj[i][(int)projection::SUM] = cv::Mat::zeros(first.size(), mattype);
    }

    // now cast proj to doubles to initialize:
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            proj[i][j].convertTo(projdbl[i][j], CV_64FC1);
        }
    }

    // initialize histograms
    histogram[0] = cv::Mat::zeros(1, 256, CV_32F);
    histogram[1] = cv::Mat::zeros(1, 256, CV_32F);
}
void VideoData::pimpl::accum(const cv::Mat& frame, bool isDff) {
    // accumulate (raw) min, max, histogram
    proj[isDff][0] = cv::min(proj[isDff][0], frame);
    proj[isDff][1] = cv::max(proj[isDff][1], frame);

    cv::accumulate(frame, projdbl[isDff][3]);
    calcHist(&frame, histogram[isDff], true);
}
void VideoData::pimpl::complete() {
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

void VideoData::pimpl::dffNativeToOrig(double& val) {
    // This helper takes my scaled dff values and translates them back into what they would be in original double space:
    void dffNativeToOrig(float& val);
    double maxval = pow(2, bitdepth); // intmax for this depth
    val = dffminval + dffrng * val / maxval;
}
cv::Mat VideoData::pimpl::calcDffDouble(const cv::Mat& frame) {
    // This calculates the df/f as a double
    cv::Mat ret(frame.size(), CV_64FC1);
    frame.convertTo(ret, CV_64FC1); // convert frame to double
    cv::subtract(ret, projdbl[0][static_cast<size_t>(VideoData::projection::MEAN)], ret);
    cv::divide(ret, projdbl[0][static_cast<size_t>(VideoData::projection::MEAN)], ret);
    return ret;
}
cv::Mat VideoData::pimpl::calcDffNative(const cv::Mat& frame) {
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

void VideoData::pimpl::readframe(size_t ind) {

    data[0][ind] = cv::Mat(height, width, mattype);

    std::string filename = files[ind].toLocal8Bit().constData();
    cv::Mat image = cv::imread(filename, cv::IMREAD_GRAYSCALE);

    if (dsSpace>1) {
        cv::resize(image, data[0][ind], data[0][ind].size(), 0, 0, cv::INTER_NEAREST);
        return;
    }

    data[0][ind]=image;
}
void VideoData::pimpl::calcHist(const cv::Mat* frame, cv::Mat& histogram, bool accum) {
    // thin wrapper on opencv calchist
    constexpr int chnl = 0;
    constexpr int histsize = 256;
    const float range[] = { 0, static_cast<float>(pow(2,bitdepth)) + 1 }; //the upper boundary is exclusive
    const float* histRange = { range };

    cv::calcHist(frame, 1, &chnl, cv::Mat(), histogram, 1, &histsize, &histRange, true, accum);
}


/*
void VideoData::computeTrace(const cv::Rect cvbb, const cv::Mat mask, const size_t row,cv::Mat &traces) {
    //todo: trying to kill this code, so remove it if I'm successful

    if (traces.empty()) {
        traces = cv::Mat(1, getNFrames(), CV_64FC1);
    }

    const int n = traces.size().height;
    if (row > n) {
        // Append n-row rows
        cv::Mat newrows = cv::Mat::zeros(row - n, traces.size().width, traces.type());
        traces.push_back(newrows);
    }

    for (size_t i = 0; i < getNFrames(); ++i) {
        cv::Mat boundedRaw = get(false, 0, i)(cvbb);
        cv::Mat boundedMu = get(false, 3, i)(cvbb);
        
        boundedRaw.convertTo(boundedRaw, CV_64FC1);
        boundedMu.convertTo(boundedMu, CV_64FC1);

        cv::Mat boundedDff = (boundedRaw - boundedMu) / boundedMu;
        double mu = cv::mean(boundedDff, mask)[0];

        traces.at<double>(row - 1, i) = mu;
    }
}
*/
cv::Mat VideoData::computeTrace(const cv::Rect cvbb, const cv::Mat mask) const {
    auto res = cv::Mat(1, getNFrames(), CV_64FC1);
    for (size_t i = 0; i < getNFrames(); ++i) {
        cv::Mat boundedRaw = get(false, 0, i)(cvbb);
        cv::Mat boundedMu = get(false, 3, i)(cvbb);
        boundedRaw.convertTo(boundedRaw, CV_64FC1);
        boundedMu.convertTo(boundedMu, CV_64FC1);
        cv::Mat boundedDff = (boundedRaw - boundedMu) / boundedMu;
        double mu = cv::mean(boundedDff, mask)[0];

        res.at<double>(0, i) = mu;
    }

    return res;
}

cv::Mat VideoData::computeTrace(ROIVert::SHAPE s, QRect bb, std::vector<QPoint> pts) const {
    // Turn the bounding box into a cv box
    const cv::Rect cvbb(static_cast<size_t>(bb.x()),
                        static_cast<size_t>(bb.y()),
                        static_cast<size_t>(bb.width()),
                        static_cast<size_t>(bb.height()));
    
    const int w = bb.width();
    const int h = bb.height();
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
    case ROIVert::SHAPE::POLYGON:
        std::vector<cv::Point> cVertices;
        for (auto& pt : pts) {
            // todo: this needs verification, it's sig. diff. from before, want to double check i didn't mess up orientation...
            cVertices.push_back(cv::Point(pt.x() - bb.left(), pt.y() - bb.top()));
        }
        cv::fillPoly(mask, cVertices, cv::Scalar(255));
        break;
    }
    
    return computeTrace(cvbb, mask);
}