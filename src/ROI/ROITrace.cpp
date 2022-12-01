#include "ROI/ROITrace.h"

#include <QDebug>
#include "opencv2/opencv.hpp"

#include "ChartStyle.h"
#include "dockwidgets/TraceViewWidget.h"
#include "ROIVertEnums.h"
#include "VideoData.h"
#include "widgets/TraceChartWidget.h"

struct ROITrace::pimpl
{
    VideoData* videodata;
    TraceViewWidget* traceview;

    cv::Mat TraceData;
    std::unique_ptr<TraceChartWidget> TraceChart;
    std::shared_ptr<TraceChartSeries> LineSeries;
    std::shared_ptr<TraceChartSeries> RidgeSeries;

    double xmin, xmax;

    void init(std::shared_ptr<ChartStyle> ridgestyle, std::shared_ptr<ChartStyle> linestyle)
    {
        TraceChart = std::make_unique<TraceChartWidget>(linestyle);
        LineSeries = std::make_shared<TraceChartSeries>(linestyle);
        RidgeSeries = std::make_shared<TraceChartSeries>(ridgestyle);
    }

    std::shared_ptr<ChartStyle> chartstyle;
};

ROITrace::ROITrace(TraceViewWidget* tv, VideoData* vd, std::shared_ptr<ChartStyle> ridgestyle, std::shared_ptr<ChartStyle> linestyle) : impl(std::make_unique<pimpl>())
{
    impl->init(ridgestyle, linestyle);
    impl->videodata = vd;
    impl->traceview = tv;
    tv->addLineChart(impl->TraceChart.get());

    impl->LineSeries->setXMax(impl->videodata->getTMax());
    impl->RidgeSeries->setXMax(impl->videodata->getTMax());

    impl->TraceChart->addSeries(impl->LineSeries);
    impl->traceview->getRidgeChart().addSeries(impl->RidgeSeries);

    connect(linestyle.get(), &ChartStyle::ColorChange, this, &ROITrace::update);
}

ROITrace::~ROITrace()
{
    if (impl->traceview)
    {
        impl->traceview->getRidgeChart().removeSeries(impl->RidgeSeries);
        impl->traceview->getRidgeChart().updateOffsets();
        impl->traceview->getRidgeChart().updateExtents();
        impl->traceview->getRidgeChart().update();
    }
}

void ROITrace::updateTrace(ROIVert::SHAPE s, QRect bb, std::vector<QPoint> pts)
{
    impl->TraceData = impl->videodata->computeTrace(s, bb, pts);
    update();
}
std::vector<float> ROITrace::getTrace() const
{
    auto numel = impl->TraceData.size().width;

    std::vector<float> out(numel, 0.);
    for (size_t i = 0; i < numel; ++i)
    {
        out[i] = impl->TraceData.at<float>(i);
    }
    return out;
}
void ROITrace::update()
{
    impl->LineSeries->setXMax(impl->videodata->getTMax());
    impl->LineSeries->setData(impl->TraceData);
    impl->TraceChart->updateExtents();
    impl->TraceChart->update();

    impl->RidgeSeries->setData(impl->TraceData);
    impl->RidgeSeries->setXMax(impl->videodata->getTMax());
    impl->traceview->getRidgeChart().updateOffsets();
    impl->traceview->getRidgeChart().updateExtents();
    impl->traceview->getRidgeChart().update();
}

TraceChartWidget* ROITrace::getTraceChart() const noexcept { return impl->TraceChart.get(); }
TraceChartSeries* ROITrace::getLineSeries() const noexcept { return impl->LineSeries.get(); }
TraceChartSeries* ROITrace::getRidgeSeries() const noexcept { return impl->RidgeSeries.get(); }