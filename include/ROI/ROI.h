#pragma once
#include "ROIShape.h"
#include "ROIStyle.h"
#include "ROITrace.h"
#include "dockwidgets/TraceViewWidget.h"
#include "VideoData.h"
#include "ChartStyle.h"

struct ROI {
    ROI(QGraphicsScene* scene,
        TraceViewWidget* tView,
        VideoData* vdata,
        ROIVert::SHAPE shp,
        QSize imgsize,
        const ROIStyle& roistyle);

    // todo: this was a first shot, but I think the goal is probably to pull a tiny bit of ROIShape out, maybe even setVertices which would possibly mean that selector could come too...
    std::unique_ptr<ROIShape> graphicsShape;
    std::shared_ptr<ROIStyle> roistyle;

    std::shared_ptr<ChartStyle> ridgechartstyle;
    std::shared_ptr<ChartStyle> linechartstyle;
    
    std::unique_ptr<ROITrace> Trace;
    std::unique_ptr<ROITrace> Ridge;
    
    int pixelsubset;

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

};
