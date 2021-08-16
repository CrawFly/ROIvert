#include "ROI\ROI.h"

ROI::ROI(QGraphicsScene* scene, TraceView* tView, VideoData* videodata, ROIVert::SHAPE shp, QSize imgsize, const ROIStyle& roistyle, const ChartStyle& chartstyle) {
    Style = std::make_unique<ROIStyle>(roistyle);

    //todo: ROIShape should take a ROIStyle, args 3-5 collapse
    graphicsShape = std::make_unique<ROIShape>(scene, shp, imgsize, roistyle);

    //todo: Trace should take a ROIStyle, it'll use that for color (for now)
    Trace = std::make_unique<ROITrace>(tView, videodata, chartstyle);

    //Chart = std::make_unique<TraceChartWidget>();
    //tView->addLineChart(Chart.get());

    // connect the shape with the chart in a way that when we update the shape the chart gets the new data...
    
}
