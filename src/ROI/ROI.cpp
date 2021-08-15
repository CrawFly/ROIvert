#include "ROI\ROI.h"

ROI::ROI(QGraphicsScene* scene, TraceView* tView, VideoData* videodata, ROIVert::SHAPE shp, QSize imgsize, const ROIStyle& style) {
    graphicsShape = std::make_unique<ROIShape>(scene, shp, imgsize, style.getSelectorSize(), style.getPen(), style.getBrush());
    Style = std::make_unique<ROIStyle>(style);

    Trace = std::make_unique<ROITrace>(tView, videodata);


    //Chart = std::make_unique<TraceChartWidget>();
    //tView->addLineChart(Chart.get());

    // connect the shape with the chart in a way that when we update the shape the chart gets the new data...
    
}
