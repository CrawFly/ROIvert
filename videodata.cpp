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
            calcHist(&dataDff->at(i), dffhistogram, true);

            // This should really be in accum...but I'm not sure if i want to continue with an accumDff or just refactor so that everything is [0] (raw) or [1] (dff)
            dffproj[(size_t)VideoData::projection::MIN] = cv::min(dffproj[(size_t)VideoData::projection::MIN], dataDff->at(i));
            dffproj[(size_t)VideoData::projection::MAX] = cv::max(dffproj[(size_t)VideoData::projection::MAX], dataDff->at(i));
            // mean is meaningless for df/f...
            emit loadProgress(50 + 50 * (float)i / data->size());
        }

    }
}

void VideoData::init() {
    data->clear();
    data->reserve(files.size());

    if (files.empty()) { return; }; // consider calling init at construction and setting first to empty mat if called with no files?

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
        dffproj[i] = first.clone();
        first.convertTo(dffprojd[i], CV_64FC1);
    }
    dffproj[2] = 0;
    // initialize histograms
    rawhistogram = cv::Mat(1, 256, CV_32F); rawhistogram = 0;
    dffhistogram = cv::Mat(1, 256, CV_32F); dffhistogram = 0;
    
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
    calcHist(&frame, rawhistogram, true);
}

void VideoData::complete() {
    int type = data->at(0).type();
    
    // Compute mean:
    projd[(size_t)VideoData::projection::MEAN] = projd[(size_t)VideoData::projection::SUM] / getNFrames();

    // (simple) Cast mean to native type, and min/max to doubles
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

cv::Mat VideoData::getProjection(VideoData::projection projtype) { return proj[(size_t)projtype]; }
cv::Mat VideoData::getProjectionDff(VideoData::projection projtype) { return dffproj[(size_t)projtype]; }
void VideoData::setStoreDff(bool enabled) { storeDff = enabled; }
bool VideoData::getStoreDff() { return storeDff; }
int VideoData::getWidth() { return width; }
int VideoData::getHeight() { return height; }
size_t VideoData::getNFrames() { return data->size(); }
int VideoData::getdsTime() { return dsTime; }
int VideoData::getdsSpace() { return dsSpace; }


void VideoData::getHistogramRaw(std::vector<float>& h) {
    // this will handle expansion just fine
    rawhistogram.copyTo(h);
}
void VideoData::getHistogramDff(std::vector<float>& h) {
    // this will handle expansion just fine
    dffhistogram.copyTo(h);
}
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
    if (isDff) {
        if (projmode == 0) { return getFrameDff(framenum); }
        else { return getProjectionDff((VideoData::projection)(projmode - 1)); }
    }
    else {
        if (projmode == 0) { return getFrameRaw(framenum); }
        else { return getProjection((VideoData::projection)(projmode - 1)); }
    }
}

void VideoData::dffNativeToOrig(double& val) {
    // This helper takes my scaled dff values and translates them back into what they would be in original double space:
    void dffNativeToOrig(float& val);
    double maxval = pow(2, bitdepth); // intmax for this depth
    val = dffminval + dffrng * val / maxval;
}