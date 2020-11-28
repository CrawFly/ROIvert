#include "videodata.h"
#include "qdebug.h"
#include <QTime>

VideoData::VideoData(QObject* parent) : QObject(parent) {
    setParent(parent);
    data->clear();
    dataDff->clear();
}
VideoData::~VideoData() {
    // destructor should delete all the Mats?
    // not sure why not..check if they're empty?
    data->clear();
    dataDff->clear();

    /*
    delete proj;
    delete projd;
    delete data;
    delete dataDff;
    */
}
void VideoData::init() {
    data->clear();
    data->reserve(files.size());

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
    rawhistogram = cv::Mat::zeros(1, 256, CV_32F); 
    dffhistogram = cv::Mat::zeros(1, 256, CV_32F); 
}

void VideoData::load(QStringList filelist, int dst, int dss){
    
    if (filelist.empty()) { return; } // should clear?

    files = filelist;
    dsTime = dst;
    dsSpace = dss;

    init();

    for (size_t i = 0; i < files.size(); i+=dsTime) {
        if (readframe(i)) { 
            accum(data->at(data->size() - 1));
        }
        emit loadProgress((100 - (storeDff * 50)) * (float)i / files.size());
    }
    complete();

    if (storeDff) {
        // when storeDff is on, compute dff for all frames and store it
        dataDff->clear();
        dataDff->reserve(data->size());
        for (size_t i = 0; i < data->size(); i++) {
            dataDff->push_back(calcDffNative(data->at(i)));
            calcHist(&dataDff->at(i), dffhistogram, true);

            // todo: This should really be in accum?
            proj[1][(size_t)VideoData::projection::MIN] = cv::min(proj[1][(size_t)VideoData::projection::MIN], dataDff->at(i));
            proj[1][(size_t)VideoData::projection::MAX] = cv::max(proj[1][(size_t)VideoData::projection::MAX], dataDff->at(i));
            // Mean has no meaning for df/f, so not accumulating sum...
            emit loadProgress(50 + 50 * (float)i / data->size());
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

    data->push_back(image.clone());
    return true;
}
void VideoData::accum(const cv::Mat &frame) {
    // accumulate (raw) min, max, histogram
    proj[0][(size_t)VideoData::projection::MIN] = cv::min(proj[0][(size_t)VideoData::projection::MIN], frame);
    proj[0][(size_t)VideoData::projection::MAX] = cv::max(proj[0][(size_t)VideoData::projection::MAX], frame);
    cv::accumulate(frame, projdbl[0][(size_t)VideoData::projection::SUM]);
    calcHist(&frame, rawhistogram, true);
}
void VideoData::complete() {
    int type = data->at(0).type();
    
    // Compute mean:
    projdbl[0][(size_t)VideoData::projection::MEAN] = projdbl[0][(size_t)VideoData::projection::SUM] / getNFrames();

    // (simple) Cast mean to native type, and min/max to doubles
    projdbl[0][(size_t)VideoData::projection::MEAN].assignTo(proj[0][(size_t)VideoData::projection::MEAN], type);
    proj[0][(size_t)VideoData::projection::MIN].assignTo(projdbl[0][(size_t)VideoData::projection::MIN], CV_64FC1);
    proj[0][(size_t)VideoData::projection::MAX].assignTo(projdbl[0][(size_t)VideoData::projection::MAX], CV_64FC1);

    // Store the range that df/f can occupy so that we can reason on it as an int
    // To do this we calculate the double df/f using the min and max projections
    // then find the scalar extrema in those, and the range
    cv::Mat mindff = calcDffDouble(projdbl[0][(size_t)VideoData::projection::MIN]);
    cv::Mat maxdff = calcDffDouble(projdbl[0][(size_t)VideoData::projection::MAX]);
    cv::minMaxLoc(mindff, &dffminval, NULL);
    cv::minMaxLoc(maxdff, NULL, &dffmaxval);
    dffrng = dffmaxval - dffminval;

    // Store the downsampled width and height:
    width = data->at(0).size().width;
    height = data->at(0).size().height;
}
cv::Mat VideoData::calcDffDouble(const cv::Mat& frame) {
    // This calculates the df/f as a double
    cv::Mat ret(frame.size(), CV_64FC1);
    frame.convertTo(ret, CV_64FC1); // convert frame to double
    cv::subtract(ret, projdbl[0][(size_t)VideoData::projection::MEAN], ret);
    cv::divide(ret, projdbl[0][(size_t)VideoData::projection::MEAN], ret);
    return ret;
}
cv::Mat VideoData::calcDffNative(const cv::Mat& frame) {
    // get the double df/f
    cv::Mat dffdbl = calcDffDouble(frame);

    // Convert to uchar, using normalization from range
    int type = data->at(0).type();
    cv::Mat ret(frame.size(), type);
    double maxval = pow(2, bitdepth);
    double alpha = maxval / dffrng;
    double beta = -1 * maxval * dffminval / dffrng;

    dffdbl.convertTo(ret, type, alpha, beta);
    return ret;
}
cv::Mat VideoData::getFrameRaw(size_t frameindex) {
    if (frameindex >= data->size()) {
        return cv::Mat();
    }
    return data->at(frameindex);
}
cv::Mat VideoData::getFrameDff(size_t frameindex) {
    if (frameindex >= data->size()) {
        return cv::Mat();
    }
    if (storeDff && frameindex>=dataDff->size()) {
        return dataDff->at(frameindex);
    }
    else {
        return calcDffNative(data->at(frameindex));
    }
}
cv::Mat VideoData::getProjection(bool isDff, VideoData::projection projtype) { return proj[isDff][(size_t)projtype]; }
void VideoData::setStoreDff(bool enabled) { storeDff = enabled; }
bool VideoData::getStoreDff() { return storeDff; }
int VideoData::getWidth() { return width; }
int VideoData::getHeight() { return height; }
size_t VideoData::getNFrames() { return data->size(); }
int VideoData::getdsTime() { return dsTime; }
int VideoData::getdsSpace() { return dsSpace; }
void VideoData::getHistogramRaw(std::vector<float>& h) {rawhistogram.copyTo(h);}
void VideoData::getHistogramDff(std::vector<float>& h) {dffhistogram.copyTo(h);}

void VideoData::getHistogramRaw(std::vector<float>& histogram, size_t framenum) {
    if (framenum < data->size()) {
        cv::Mat hist;
        calcHist(&data->at(framenum), hist, false);
        hist.copyTo(histogram);
    }
}
void VideoData::getHistogramDff(std::vector<float>& histogram, size_t framenum) {
    if (framenum < dataDff->size()) {
        cv::Mat hist;
        calcHist(&dataDff->at(framenum), hist, false);
        hist.copyTo(histogram);
    }
}

void VideoData::calcHist(const cv::Mat* frame, cv::Mat& histogram, bool accum) {
    // thin wrapper on opencv calchist
    const int chnl = 0;
    const int histsize = 256;
    float range[] = { 0, pow(2,bitdepth) + 1 }; //the upper boundary is exclusive
    const float* histRange = { range };

    cv::calcHist(frame, 1, &chnl, cv::Mat(), histogram, 1, &histsize, &histRange, true, accum);
}
cv::Mat VideoData::get(bool isDff, int projmode, size_t framenum) {
    if (projmode > 0) {
        return getProjection(isDff, (VideoData::projection)(projmode - 1));
    }

    if (isDff) {
        if (projmode == 0) { return getFrameDff(framenum); }
    }
    else {
        if (projmode == 0) { return getFrameRaw(framenum); }
    }
}
void VideoData::dffNativeToOrig(double& val) {
    // This helper takes my scaled dff values and translates them back into what they would be in original double space:
    void dffNativeToOrig(float& val);
    double maxval = pow(2, bitdepth); // intmax for this depth
    val = dffminval + dffrng * val / maxval;
}

std::vector<double> VideoData::calcTrace(cv::Rect cvbb, cv::Mat mask) {
    std::vector<double> trace;
    trace.reserve(dataDff->size());
    QTime t;
    t.start();
    for (size_t i = 0; i < dataDff->size(); i++) {
        cv::Mat boundedimage = dataDff->at(i)(cvbb);
        double mu = cv::mean(boundedimage, mask)[0];
        dffNativeToOrig(mu);
        trace.push_back(mu);
    }
    return trace;
}