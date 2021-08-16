#pragma once
#include "ROIShape.h"
#include "ROIStyle.h"
#include "ROITrace.h"
#include "TraceView.h"
#include "videodata.h"
#include "ChartStyle.h"

struct ROI {
    ROI(QGraphicsScene* scene,
        TraceView* tView,
        VideoData* vdata,
        ROIVert::SHAPE shp,
        QSize imgsize,
        const ROIStyle& roistyle,
        const ChartStyle& chartstyle);

    // todo: this was a first shot, but I think the goal is probably to pull a tiny bit of ROIShape out, maybe even setVertices which would possibly mean that selector could come too...
    std::unique_ptr<ROIShape> graphicsShape;
    std::unique_ptr<ROIStyle> Style;
    
    std::unique_ptr<ROITrace> Trace;
    std::unique_ptr<ROITrace> Ridge;
    //std::unique_ptr<TraceChartWidget> Chart;
};
