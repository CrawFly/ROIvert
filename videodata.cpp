#include "videodata.h"
#include "qdebug.h"

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

void VideoData::load(QStringList filelist, int dst, int dss){
    
    if (filelist.empty()) { return; } // should clear?

    files = filelist;
    dsTime = dst;
    dsSpace = dss;

    init();

    for (size_t i = 1; i < files.size(); i+=dsTime) {
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
            emit loadProgress(50 + 50 * (float)i / data->size());
        }
    }
}

void VideoData::init() {
    if (files.empty()) { return; };

    data->clear();
    data->reserve(files.size());

    // Read first frame for info
    cv::Mat first = cv::imread(files[0].toLocal8Bit().constData(), cv::IMREAD_GRAYSCALE);

    // Store size
    width = first.size().width;
    height = first.size().height;

    if (dsSpace > 1) {
        // downsample for space
        cv::resize(first, first, cv::Size(), 1. / dsSpace, 1. / dsSpace, cv::INTER_NEAREST);
    }

    // Store bit depth (only allow 8 and 16 right now)!
    if (first.depth() == CV_8U) { bitdepth = 8; }
    else if (first.depth() == CV_16U) { bitdepth = 16; }
    else { qFatal("Bad file type"); }
    
    // initialize projections
    for (int i = 0; i < 4; i++) {
        proj[i] = first.clone();
        first.convertTo(projd[i], CV_64FC1);
    }
    data->push_back(first.clone());
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
    proj[(size_t)VideoData::projection::MIN] = cv::min(proj[(size_t)VideoData::projection::MIN], frame);
    proj[(size_t)VideoData::projection::MAX] = cv::max(proj[(size_t)VideoData::projection::MAX], frame);
    cv::accumulate(frame, projd[(size_t)VideoData::projection::SUM]);
}

void VideoData::complete() {
    int type = data->at(0).type();
    
    // Compute mean:
    projd[(size_t)VideoData::projection::MEAN] = projd[(size_t)VideoData::projection::SUM] / getNFrames();

    // (simple) Cast mean to nativy type, and min/max to doubles
    projd[(size_t)VideoData::projection::MEAN].assignTo(proj[(size_t)VideoData::projection::MEAN], type);
    proj[(size_t)VideoData::projection::MIN].assignTo(projd[(size_t)VideoData::projection::MIN], CV_64FC1);
    proj[(size_t)VideoData::projection::MAX].assignTo(projd[(size_t)VideoData::projection::MAX], CV_64FC1);

    // Store the range that df/f can occupy so that we can reason on it as an int
    // To do this we calculate the double df/f using the min and max projections
    // then find the scalar extrema in those, and the range
    cv::Mat mindff = calcDffDouble(projd[(size_t)VideoData::projection::MIN]);
    cv::Mat maxdff = calcDffDouble(projd[(size_t)VideoData::projection::MAX]);
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
    cv::subtract(ret, projd[(size_t)VideoData::projection::MEAN], ret);
    cv::divide(ret, projd[(size_t)VideoData::projection::MEAN], ret);
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

cv::Mat VideoData::getProjection(VideoData::projection projtype) {return proj[(size_t)projtype];}
void VideoData::setStoreDff(bool enabled) { storeDff = enabled; }
bool VideoData::getStoreDff() { return storeDff; }
int VideoData::getWidth() { return width; }
int VideoData::getHeight() { return height; }
size_t VideoData::getNFrames() { return data->size(); }

