#include "tracecomputer.h"
#include "opencv2/opencv.hpp"
#include <QThread>;

namespace {
    cv::Rect qt2cv_rect(QRect r) {
        return cv::Rect((size_t)r.x(), size_t(r.y()), (size_t)r.width(), (size_t)r.height());;
    }
}


void TraceComputer::update(ImageROIViewer* imView, VideoData* imData, const int ind) {
    
    if (ind > traces.size()+1) {
        return;
    }
    // todo: would be good to have some locking here
    //      Case: roi is deleted while the trace is being updated. mostly fine as roi gets queried outside of the loop?
    //      Case: video load while the trace is being updated. frame will get pulled in as an empty. cv might stb...

    if (ind == traces.size()) {
        std::vector<double> newtrace;
        traces.push_back(newtrace);
    }
    
    traces[ind].clear();
    traces[ind].resize(imData->getNFrames());

    roi* thisroi = imView->getRoi(ind);
    cv::Mat thismask = thisroi->getMask();
    cv::Rect thisbox = qt2cv_rect(thisroi->getBB());
    
    for (size_t i = 0; i < imData->getNFrames(); i++) {

        //** this is df/f, but the units are wrong.
        // I need to scale it back to where it should be as a double...

        cv::Mat frame = imData->getFrameDff(i);
        double val = cv::mean(frame(thisbox), thismask)[0];
        imData->dffNativeToOrig(val);
        traces[ind][i] = val;
    }

    emit traceComputed(ind + 1, traces[ind]);
}   

void TraceComputer::updateall(ImageROIViewer* imView, VideoData* imData) {
    // This will update all traces (more efficient that iterating over update(ind)) because it iterates over rois on the inner loop
    // Note that trace computes are expensive!

}              
std::vector<double> TraceComputer::getTrace(size_t ind){
    return traces[ind];
}
