#include "videodata.h"
#include "qdebug.h"
#include <QTime>

VideoData::VideoData(QObject* parent) : QObject(parent) {
    setParent(parent);
    data[0]->clear();
    data[1]->clear();
}
VideoData::~VideoData() {
    // bug: destructor should be deleting all the MATs, but they're not init'd correctly (?)
    data[0]->clear();
    data[1]->clear();
}
void VideoData::init() {
    data[0]->clear();
    data[0]->reserve(files.size());

    // todo: consider calling init at construction and setting first to empty mat if called with no files?
    if (files.empty()) { return; }; 

    // Read first frame for some metadata:
    cv::Mat first = cv::imread(files[0].toLocal8Bit().constData(), cv::IMREAD_GRAYSCALE);

    // Store raw size:
    width = first.size().width;
    height = first.size().height;

    // Store bit depth (only allow 8 and 16 right now)!
    if (first.depth() == CV_8U) { bitdepth = 8; }
    else if (first.depth() == CV_16U) { bitdepth = 16; }
    else { qFatal("Bad file type"); }

    if (dsSpace > 1) {
        // Do spatial downsampling, important to do this before initializing projections
        cv::resize(first, first, cv::Size(), 1. / dsSpace, 1. / dsSpace, cv::INTER_NEAREST);
    }

    // initialize projections
    for (int i = 0; i < 2; i++) { // looping over raw and dff
        proj[i][(int)projection::MIN] = first.clone(); // initialize min and max as the first frame, will compare on each load
        proj[i][(int)projection::MAX] = first.clone(); 
        proj[i][(int)projection::MEAN] = cv::Mat::zeros(first.size(), first.type());
        proj[i][(int)projection::SUM] = cv::Mat::zeros(first.size(), first.type());
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

void VideoData::load(QStringList filelist, int dst, int dss){
    
    if (filelist.empty()) { return; } // should clear?

    files = filelist;
    dsTime = dst;
    dsSpace = dss;

    init();
    for (size_t i = 0; i < files.size(); i+=dsTime) {
        if (readframe(i)) {
            accum(data[0]->back());
        }
        emit loadProgress((100 - (storeDff * 50)) * (float)i / files.size());
    }
    //qDebug() << t_readframe << t_accum; // 2361 + 627
    
    complete();

    if (storeDff) {
        // when storeDff is on, compute dff for all frames and store it
        data[1]->clear();
        data[1]->reserve(data[0]->size());
        for (size_t i = 0; i < data[0]->size(); i++) {
            data[1]->push_back(calcDffNative(data[0]->operator[](i)));
            calcHist(&(data[1]->back()), histogram[1], true);

            // todo: This should really be in accum?
            proj[1][static_cast<size_t>(VideoData::projection::MIN)] = 
                cv::min(proj[1][static_cast<size_t>(VideoData::projection::MIN)], data[1]->at(i));
            proj[1][static_cast<size_t>(VideoData::projection::MAX)] = 
                cv::max(proj[1][static_cast<size_t>(VideoData::projection::MAX)], data[1]->at(i));

            // Mean has no meaning for df/f, so not accumulating sum...
            emit loadProgress(50 + 50 * (float)i / data[0]->size());
        }

    }
}
bool VideoData::readframe(size_t filenum) {
    std::string filename = files[filenum].toLocal8Bit().constData();
    cv::Mat image = cv::imread(filename, cv::IMREAD_GRAYSCALE);

    if (image.size().width != width || image.size().height != height ) { // could add a check for bit depth here!
        return false;
    }

    if (dsSpace>1) {
        // downsample for space
        cv::resize(image, image, cv::Size(), 1. / dsSpace, 1. / dsSpace, cv::INTER_NEAREST);
    }

    //data[0]->push_back(image.clone());
    data[0]->push_back(image);
    return true;
}
void VideoData::accum(const cv::Mat &frame) {
    // accumulate (raw) min, max, histogram
    proj[0][static_cast<size_t>(VideoData::projection::MIN)] = cv::min(proj[0][static_cast<size_t>(VideoData::projection::MIN)], frame);
    proj[0][static_cast<size_t>(VideoData::projection::MAX)] = cv::max(proj[0][static_cast<size_t>(VideoData::projection::MAX)], frame);
    cv::accumulate(frame, projdbl[0][static_cast<size_t>(VideoData::projection::SUM)]);
    calcHist(&frame, histogram[0], true);
}
void VideoData::complete() {
    int type = data[0]->at(0).type();
    
    // Compute mean:
    projdbl[0][static_cast<size_t>(VideoData::projection::MEAN)] = projdbl[0][static_cast<size_t>(VideoData::projection::SUM)] / getNFrames();

    // (simple) Cast mean to native type, and min/max to doubles
    projdbl[0][static_cast<size_t>(VideoData::projection::MEAN)].assignTo(proj[0][static_cast<size_t>(VideoData::projection::MEAN)], type);
    proj[0][static_cast<size_t>(VideoData::projection::MIN)].assignTo(projdbl[0][static_cast<size_t>(VideoData::projection::MIN)], CV_64FC1);
    proj[0][static_cast<size_t>(VideoData::projection::MAX)].assignTo(projdbl[0][static_cast<size_t>(VideoData::projection::MAX)], CV_64FC1);

    // Store the range that df/f can occupy so that we can reason on it as an int
    // To do this we calculate the double df/f using the min and max projections
    // then find the scalar extrema in those, and the range
    cv::Mat mindff = calcDffDouble(projdbl[0][static_cast<size_t>(VideoData::projection::MIN)]);
    cv::Mat maxdff = calcDffDouble(projdbl[0][static_cast<size_t>(VideoData::projection::MAX)]);
    cv::minMaxLoc(mindff, &dffminval, NULL);
    cv::minMaxLoc(maxdff, NULL, &dffmaxval);
    dffrng = dffmaxval - dffminval;

    // Store the downsampled width and height:
    width = data[0]->at(0).size().width;
    height = data[0]->at(0).size().height;
}
cv::Mat VideoData::calcDffDouble(const cv::Mat& frame) {
    // This calculates the df/f as a double
    cv::Mat ret(frame.size(), CV_64FC1);
    frame.convertTo(ret, CV_64FC1); // convert frame to double
    cv::subtract(ret, projdbl[0][static_cast<size_t>(VideoData::projection::MEAN)], ret);
    cv::divide(ret, projdbl[0][static_cast<size_t>(VideoData::projection::MEAN)], ret);
    return ret;
}
cv::Mat VideoData::calcDffNative(const cv::Mat& frame) {
    // get the double df/f
    cv::Mat dffdbl = calcDffDouble(frame);

    // Convert to uchar, using normalization from range
    const int type = data[0]->at(0).type();
    cv::Mat ret(frame.size(), type);
    const double maxval = pow(2, bitdepth);
    const double alpha = maxval / dffrng;
    const double beta = -1 * maxval * dffminval / dffrng;

    dffdbl.convertTo(ret, type, alpha, beta);
    return ret;
}

cv::Mat VideoData::getFrame(bool isDff, size_t frameindex) {
    if (frameindex >= data[isDff]->size()) {
        return cv::Mat();
    }
    return data[isDff]->operator[](frameindex);
}
/* Old getFrame Impl...will allow cacheless version?
cv::Mat VideoData::getFrameRaw(size_t frameindex) {
    if (frameindex >= data[0]->size()) {
    }
    return data[0]->at(frameindex);
}
cv::Mat VideoData::getFrameDff(size_t frameindex) {
    if (frameindex >= data[1]->size()) {
        return cv::Mat();
    }
    if (storeDff && frameindex>= data[1]->size()) {
        return data[1]->at(frameindex);
    }
    else {
        return calcDffNative(data[0]->at(frameindex));
    }
}
*/


cv::Mat VideoData::getProjection(bool isDff, VideoData::projection projtype) { return proj[isDff][(size_t)projtype]; }
void VideoData::setStoreDff(bool enabled) { storeDff = enabled; }
bool VideoData::getStoreDff() { return storeDff; }
int VideoData::getWidth() { return width; }
int VideoData::getHeight() { return height; }
size_t VideoData::getNFrames() { return data[0]->size(); }
int VideoData::getdsTime() { return dsTime; }
int VideoData::getdsSpace() { return dsSpace; }
void VideoData::getHistogram(bool isDff, std::vector<float>& h) {histogram[isDff].copyTo(h);}

void VideoData::getHistogram(bool isDff, std::vector<float>& histogram, size_t framenum) {
    if (framenum < getNFrames()) {
        cv::Mat hist;
        // TODO: drop if when data is collapsed
        calcHist(&data[isDff]->at(framenum), hist, false);
        hist.copyTo(histogram);
    }
}

void VideoData::calcHist(const cv::Mat* frame, cv::Mat& histogram, bool accum) {
    // thin wrapper on opencv calchist
    constexpr int chnl = 0;
    constexpr int histsize = 256;
    const float range[] = { 0, static_cast<float>(pow(2,bitdepth)) + 1 }; //the upper boundary is exclusive
    const float* histRange = { range };

    cv::calcHist(frame, 1, &chnl, cv::Mat(), histogram, 1, &histsize, &histRange, true, accum);
}
cv::Mat VideoData::get(bool isDff, int projmode, size_t framenum) {
    // This is just a convenience that wraps getProjection and getFrame.
    if (projmode > 0) {
        return getProjection(isDff, static_cast<VideoData::projection>(projmode - 1));
    }
    return getFrame(isDff, framenum);
}
void VideoData::dffNativeToOrig(double& val) {
    // This helper takes my scaled dff values and translates them back into what they would be in original double space:
    void dffNativeToOrig(float& val);
    double maxval = pow(2, bitdepth); // intmax for this depth
    val = dffminval + dffrng * val / maxval;
}

std::vector<double> VideoData::calcTrace(cv::Rect cvbb, cv::Mat mask) {
    // todo: allow isDff bool in here so that we can calculate traces for raw data
    std::vector<double> trace;
    trace.reserve(getNFrames());
    QTime t;
    t.start();
    for (size_t i = 0; i < getNFrames(); i++) {
        cv::Mat boundedimage = data[1]->at(i)(cvbb);
        double mu = cv::mean(boundedimage, mask)[0];
        dffNativeToOrig(mu);
        trace.push_back(mu);
    }
    return trace;
}