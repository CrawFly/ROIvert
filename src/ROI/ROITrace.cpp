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

    void init(std::shared_ptr<ChartStyle> style) {
        chartstyle = style;
        TraceChart = std::make_unique<TraceChartWidget>(chartstyle);
        LineSeries = std::make_shared<TraceChartSeries>(chartstyle);
        RidgeSeries = std::make_shared<TraceChartSeries>(chartstyle);
    }

    std::shared_ptr<ChartStyle> chartstyle;
};


ROITrace::ROITrace(TraceView* tv, VideoData* vd, std::shared_ptr<ChartStyle> style) {
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
std::vector<float> ROITrace::getTrace() const {
    auto numel = impl->TraceData.size().width;
    
    std::vector<float> out(numel, 0.);
    for (size_t i = 0; i < numel; ++i) {
        out[i] = impl->TraceData.at<float>(i);
    }
    return out;
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

TraceChartWidget* ROITrace::getTraceChart() const noexcept { return impl->TraceChart.get(); }
TraceChartSeries* ROITrace::getLineSeries() const noexcept { return impl->LineSeries.get();  }
TraceChartSeries* ROITrace::getRidgeSeries() const noexcept { return impl->RidgeSeries.get(); }
