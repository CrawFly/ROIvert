#include "roi\ROITrace.h"
#include <QDebug>

#include "videodata.h"
#include "ROIVertEnums.h"
#include "TraceView.h"
#include "widgets\TraceChartWidget.h"
#include "widgets\RidgeLineWidget.h"
#include "opencv2/opencv.hpp"
#include "ChartStyle.h"

struct ROITrace::pimpl {
    VideoData* videodata;
    TraceView* traceview;

    cv::Mat TraceData;
    std::unique_ptr<TraceChartWidget> TraceChart;
    std::shared_ptr<TraceChartSeries> LineSeries;
    std::shared_ptr<TraceChartSeries> RidgeSeries;

    double xmin, xmax;

    void init(const ChartStyle &style) {
        TraceChart = std::make_unique<TraceChartWidget>();
        LineSeries = std::make_shared<TraceChartSeries>(style);
        RidgeSeries = std::make_shared<TraceChartSeries>(style);
    }
};


ROITrace::ROITrace(TraceView* tv, VideoData* vd, const ChartStyle &style) {
    impl->init(style);
    impl->videodata = vd;
    impl->traceview = tv;
    tv->addLineChart(impl->TraceChart.get());

    impl->LineSeries->setXMax(impl->videodata->getTMax());
    impl->RidgeSeries->setXMax(impl->videodata->getTMax());

    impl->TraceChart->addSeries(impl->LineSeries);
    impl->traceview->getRidgeChart().addSeries(impl->RidgeSeries);

    //impl->LineSeries->setColor(style.getPen().color());
    //impl->RidgeSeries->setColor(style.getPen().color());
}

ROITrace::~ROITrace() {
    if (impl->traceview) {
        impl->traceview->getRidgeChart().removeSeries(impl->RidgeSeries);
        impl->traceview->getRidgeChart().updateOffsets();
        impl->traceview->getRidgeChart().updateExtents();
        impl->traceview->getRidgeChart().update();
    }
}

void ROITrace::updateTrace(ROIVert::SHAPE s, QRect bb, std::vector<QPoint> pts) {
    impl->TraceData = impl->videodata->computeTrace(s, bb, pts);
    update();
}

void ROITrace::update() {
    impl->LineSeries->setXMax(impl->videodata->getTMax());
    impl->LineSeries->setData(impl->TraceData);
    impl->TraceChart->updateExtents();
    impl->TraceChart->update();

    impl->RidgeSeries->setData(impl->TraceData, ROIVert::NORMALIZATION::ZEROTOONE);
    impl->RidgeSeries->setXMax(impl->videodata->getTMax());
    impl->traceview->getRidgeChart().updateOffsets();
    impl->traceview->getRidgeChart().updateExtents();
    impl->traceview->getRidgeChart().update();
}