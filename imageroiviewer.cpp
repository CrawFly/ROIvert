#include "imageroiviewer.h"
#include <QMouseEvent>
#include <qdebug.h>
#include <QTime>

namespace
{
    bool clickOutOfBounds(const QPointF clickpos, const QSize imgsize)
    {
        return clickpos.x() < 0 || clickpos.x() > imgsize.width() - 1 || clickpos.y() < 0 || clickpos.y() > imgsize.height() - 1;
    }
    void clickToBounds(QPointF &clickpos, const QSize imgsize)
    {
        clickpos.setX(qMax(0., qMin(clickpos.x(), (qreal)imgsize.width() - 1)));
        clickpos.setY(qMax(0., qMin(clickpos.y(), (qreal)imgsize.height() - 1)));
    }
} // namespace
ImageROIViewer::ImageROIViewer(QWidget *parent)
    : QGraphicsView(parent)
{
    scene = new QGraphicsScene(parent);
    pixitem = new QGraphicsPixmapItem();
    scene->addItem(pixitem);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setScene(scene);
    setBackgroundBrush(Qt::black);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->setParent(parent);
}
ImageROIViewer::~ImageROIViewer()
{
}

// Image Stuff
void ImageROIViewer::setImage(const QImage image)
{
    pixitem->setPixmap(QPixmap::fromImage(image));
    QSize oldSize = img.size();
    QSize newSize = image.size();
    img = image;

    hasImage = true;
    emit imgLoaded();

    if (oldSize != newSize)
    {
        fitInView(pixitem, Qt::KeepAspectRatio);
        emit imgSizeChanged(newSize);
    }
}
QImage ImageROIViewer::getImage()
{
    return img;
}
QSize ImageROIViewer::getImageSize()
{
    return img.size();
}

// ROI stuff:
void ImageROIViewer::pushROI()
{
    switch (roishape)
    {
    case ROIVert::RECTANGLE:
    {
        rois.push_back(new roi_rect(scene));
        break;
    }
    case ROIVert::ELLIPSE:
    {
        rois.push_back(new roi_ellipse(scene));
        break;
    }
    case ROIVert::POLYGON:
    {
        rois.push_back(new roi_polygon(scene));
        break;
    }
    }
}

// Mouse/resize stuff stuff:
void ImageROIViewer::resizeEvent(QResizeEvent *event)
{
    fitInView(pixitem, Qt::KeepAspectRatio);
}
void ImageROIViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
    {
        return; // skip right clicks for now
    }

    QPointF clickpos = mapToScene(event->pos());
    if (clickOutOfBounds(clickpos, getImageSize()))
    {
        return;
    }

    if (mousestatus.mode == ROIVert::ADDROI)
    {

        if (mousestatus.ActiveROI == 0)
        {
            // Add mode with no active ROI, push one and set it
            pushROI();
            mousestatus.ActiveROI = rois.size(); // Note ActiveROI uses 1 based indexing
            QVector<QPoint> pt = {clickpos.toPoint(), clickpos.toPoint()};
            mousestatus.ActiveVert = 2; // Also 1 based indexing.
            rois[mousestatus.ActiveROI - 1]->setVertices(pt);
        }
        else if (roishape == ROIVert::POLYGON && mousestatus.ActiveVert > 0)
        {
            // this should be the only other way i get here?
            QVector<QPoint> pt = rois[mousestatus.ActiveROI - 1]->getVertices();
            pt.push_back(clickpos.toPoint());
            mousestatus.ActiveVert++;
            rois[mousestatus.ActiveROI - 1]->setVertices(pt);
        }
    }
}
void ImageROIViewer::mouseMoveEvent(QMouseEvent *event)
{
    // watch out need a check here that we started a vertex move, because we don't get a button here...
    if (mousestatus.ActiveROI == 0 || mousestatus.ActiveVert == 0)
    {
        return;
    };

    QPointF clickpos = mapToScene(event->pos());
    clickToBounds(clickpos, getImageSize());

    size_t ROIind = mousestatus.ActiveROI - 1;
    size_t VERTind = mousestatus.ActiveVert - 1;

    QVector<QPoint> verts = rois[ROIind]->getVertices();
    verts[VERTind] = clickpos.toPoint();
    rois[ROIind]->setVertices(verts);
}
void ImageROIViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (mousestatus.ActiveROI == 0 || mousestatus.ActiveVert == 0)
    {
        return;
    };
    if (mousestatus.mode == ROIVert::ADDROI && roishape != ROIVert::POLYGON)
    {
        mousestatus.ActiveROI = 0;
        mousestatus.ActiveVert = 0;
    }
}
void ImageROIViewer::mouseDoubleClickEvent(QMouseEvent *event)
{

    if (event->button() != Qt::LeftButton)
    {
        return; // skip right clicks for now
    }

    if (mousestatus.mode == ROIVert::ADDROI && mousestatus.ActiveROI > 0 && mousestatus.ActiveVert > 0 && roishape == ROIVert::POLYGON)
    {
        // Special case for polygons, we finish it with a double click.
        // That means popping off the last added vertex (from the first click of the double):
        QVector<QPoint> pt = rois[mousestatus.ActiveROI - 1]->getVertices();
        pt.pop_back();
        rois[mousestatus.ActiveROI - 1]->setVertices(pt);
        mousestatus.ActiveROI = 0;
        mousestatus.ActiveVert = 0;
    }
}
void ImageROIViewer::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "Key Press";
    qDebug() << event->key();
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && mousestatus.mode == ROIVert::ADDROI && mousestatus.ActiveROI > 0 && mousestatus.ActiveVert > 0)
    {
        // Special case for polygons, we finish it with an enter:
        mousestatus.ActiveROI = 0;
        mousestatus.ActiveVert = 0;
    }
}
void ImageROIViewer::wheelEvent(QWheelEvent *event)
{
    int y = event->angleDelta().y();

    // see https://doc.qt.io/qt-5/qwheelevent.html#angleDelta - can support high precision mouse scroll
    if (y > 0)
    {
        qDebug() << "Wheel in";
    }
    else if (y < 0)
    {
        qDebug() << "Wheel out";
    }
}

// Interfaces for UI:
void ImageROIViewer::setMouseMode(ROIVert::MODE md)
{
    mousestatus.mode = md;
}
void ImageROIViewer::setROIShape(ROIVert::ROISHAPE shp)
{
    roishape = shp;
}