#include "roi\ROITrace.h"
#include <QDebug>

#include "videodata.h"
#include "ROIVertEnums.h"
#include "TraceView.h"
#include "widgets\TraceChartWidget.h"
#include "widgets\RidgeLineWidget.h"
#include "opencv2/opencv.hpp"

struct ROITrace::pimpl {
    VideoData* vd;
    TraceView* tv;

    cv::Mat TraceData;
    std::unique_ptr<TraceChartWidget> tc = std::make_unique<TraceChartWidget>();
    std::shared_ptr<TraceChartSeries> LineSeries = std::make_shared<TraceChartSeries>();
    std::shared_ptr<TraceChartSeries> RidgeSeries = std::make_shared<TraceChartSeries>();
};


ROITrace::ROITrace(TraceView* tv, VideoData* vd) {
    impl->vd = vd;
    impl->tv = tv;
    tv->addLineChart(impl->tc.get());

    // todo: need the limits for the chart
    impl->tc->addSeries(impl->LineSeries);
    impl->tv->getRidgeChart().addSeries(impl->RidgeSeries);
}
ROITrace::~ROITrace() {
    if (impl->tv) {
        impl->tv->getRidgeChart().removeSeries(impl->RidgeSeries);
        impl->tv->getRidgeChart().updateOffsets();
        impl->tv->getRidgeChart().updateExtents();
        impl->tv->getRidgeChart().update();
    }
}

void ROITrace::updateTrace(ROIVert::SHAPE s, QRect bb, std::vector<QPoint> pts) {
    impl->TraceData = impl->vd->computeTrace(s, bb, pts);
    impl->LineSeries->setData(impl->TraceData);
    impl->tc->updateExtents();
    impl->tc->update();

    impl->RidgeSeries->setData(impl->TraceData, ROIVert::NORMALIZATION::ZEROTOONE);
    impl->tv->getRidgeChart().updateOffsets();
    impl->tv->getRidgeChart().updateExtents();
    impl->tv->getRidgeChart().update();
}