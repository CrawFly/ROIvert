#pragma once
#include "ROIShape.h"
#include "ROIStyle.h"

struct ROI {
    ROI(QGraphicsScene* scene,
        ROIVert::SHAPE shp,
        QSize imgsize,
        const ROIStyle& style);

    // todo: this was a first shot, but I think the goal is probably to pull a tiny bit of ROIShape out, maybe even setVertices which would possibly mean that selector could come too...
    std::unique_ptr<ROIShape> graphicsShape;
    std::unique_ptr<ROIStyle> Style;
};
