#include "ROI\ROI.h"

ROI::ROI(QGraphicsScene* scene, TraceView* tView, VideoData* videodata, ROIVert::SHAPE shp, QSize imgsize, const ROIStyle& rstyle, const ChartStyle& cstyle) {
    
    roistyle = std::make_unique<ROIStyle>(rstyle);
    chartstyle = std::make_unique<ChartStyle>(cstyle);
    chartstyle->connectToROIStyle(roistyle.get()); // is this safe? I think so, the signal can't fire during destruction...

    graphicsShape = std::make_unique<ROIShape>(scene, shp, imgsize, roistyle); 
    Trace = std::make_unique<ROITrace>(tView, videodata, chartstyle);
    
}
