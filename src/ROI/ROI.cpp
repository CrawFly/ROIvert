#include "ROI\ROI.h"
#include <QJsonObject>
#include <QJsonArray>

ROI::ROI(QGraphicsScene* scene, TraceViewWidget* tView, VideoData* videodata, ROIVert::SHAPE shp, QSize imgsize, const ROIStyle& rstyle) {
    
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

    
    QJsonArray jrgb = jShape["RGB"].toArray();
    const QColor clr(jrgb[0].toInt(), jrgb[1].toInt(), jrgb[2].toInt());
    roistyle->setColor(clr);
}
void ROI::write(QJsonObject& json) const {
    QJsonObject jShape;
    graphicsShape->write(jShape, pixelsubset);
    
    const auto clr = roistyle->getLineColor();
    QJsonArray rgb;
    rgb.append(clr.red());
    rgb.append(clr.green());
    rgb.append(clr.blue());
    jShape["RGB"] = rgb;
    json["shape"] = jShape;
}