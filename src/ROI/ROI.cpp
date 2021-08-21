#include "ROI\ROI.h"
#include <QJsonObject>

ROI::ROI(QGraphicsScene* scene, TraceView* tView, VideoData* videodata, ROIVert::SHAPE shp, QSize imgsize, const ROIStyle& rstyle, const ChartStyle& cstyle) {
    
    roistyle = std::make_unique<ROIStyle>(rstyle);
    chartstyle = std::make_unique<ChartStyle>(cstyle);
    chartstyle->connectToROIStyle(roistyle.get()); 

    graphicsShape = std::make_unique<ROIShape>(scene, shp, imgsize, roistyle); 
    Trace = std::make_unique<ROITrace>(tView, videodata, chartstyle);
    
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

    // todo: style read write
    //QJsonObject jroistyle;
    //roistyle->write(jroistyle);
    //json["roistyle"]=jroistyle;
    
    // roistyle.write(json);
    // chartstyle.write(json);
}