#include "ROI\ROISelector.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QCursor>
#include <QDebug>

struct ROISelector::pimpl {
    QVector<QPoint> verts;
    QPainterPath path;
    QRectF bb;

    QColor Color;
    double sz = 20.;
};

ROISelector::ROISelector(ROIShape* par){
    setParentItem(par);
    setCursor(Qt::SizeAllCursor);
    setZValue(1);
}

ROISelector::~ROISelector() = default;

void ROISelector::setVertices(const QVector<QPoint> verts){
    prepareGeometryChange();
    impl->verts = verts;
}

void ROISelector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    const double scale = scene()->views()[0]->transform().m11();
    const double sz = impl->sz / scale;
    
    impl->path.clear();
    for (const auto& vert : impl->verts) {
        impl->path.addRect(vert.x() - sz/2, vert.y() - sz/2, sz, sz);
    }

    painter->setPen(QPen(impl->Color, impl->sz/scale));
    painter->drawPoints(impl->verts);
    painter->setPen(QPen(Qt::lightGray, impl->sz/(2*scale)));
    painter->drawPoints(impl->verts);
}

QRectF ROISelector::boundingRect() const { 
    
    // clamp path rect to image bounding box
    //auto intersectrect = impl->path.boundingRect() & impl->bb;
    //return intersectrect;

    // determining this based on the path has a problem that in some circumstances we don't paint because there's no path??
    return impl->bb;
}

int ROISelector::type() const { 
    return Type;
}

QPainterPath ROISelector::shape() const { 
    if (isVisible()) {
        return impl->path; 
    }
    return QPainterPath();
}
void ROISelector::setColor(const QColor clr) { impl->Color = clr; }
void ROISelector::setBoundingRect(QRectF bb) { impl->bb = bb; }




void ROISelector::setSize(double sz) {
    impl->sz = sz;
}