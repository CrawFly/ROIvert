#include "ROI\ROI.h"
#include <QJsonObject>

ROI::ROI(QGraphicsScene* scene, TraceView* tView, VideoData* videodata, ROIVert::SHAPE shp, QSize imgsize, const ROIStyle& rstyle) {
    
    roistyle = std::make_unique<ROIStyle>(rstyle);

    auto rcs = tView->getCoreRidgeChartStyle();
    auto lcs = tView->getCoreLineChartStyle();

    ridgechartstyle = std::make_unique<ChartStyle>(*rcs);
    linechartstyle = std::make_unique<ChartStyle>(*lcs);

    ridgechartstyle->connectToROIStyle(roistyle.get()); 
    linechartstyle->connectToROIStyle(roistyle.get()); 

    graphicsShape = std::make_unique<ROIShape>(scene, shp, imgsize, roistyle); 
    Trace = std::make_unique<ROITrace>(tView, videodata, ridgechartstyle, linechartstyle);
    
    pixelsubset = videodata->getdsSpace();
}

void ROI::read(const QJsonObject& json) {
    QJsonObject jShape = json["shape"].toObject();
    
    graphicsShape->read(jShape, pixelsubset);
    // QJsonObject roistyle = roistyle.read
}
void ROI::write(QJsonObject& json) const {
    QJsonObject jShape;
    graphicsShape->write(jShape, pixelsubset);
    json["shape"] = jShape;

    // todo: style read write *** really just color
    //QJsonObject jroistyle;
    //roistyle->write(jroistyle);
    //json["roistyle"]=jroistyle;
    
    // roistyle.write(json);
    // chartstyle.write(json);
}