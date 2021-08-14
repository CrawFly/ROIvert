#include "ROI\ROI.h"

ROI::ROI(QGraphicsScene* scene, ROIVert::SHAPE shp, QSize imgsize, const ROIStyle& style) {
    graphicsShape = std::make_unique<ROIShape>(scene, shp, imgsize, style.getSelectorSize(), style.getPen(), style.getBrush());
    Style = std::make_unique<ROIStyle>(style);
}
