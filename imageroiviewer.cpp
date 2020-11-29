#include "imageroiviewer.h"
#include <QMouseEvent>
#include <qdebug.h>
#include <QTime>
#include <qapplication.h>
#include <qmath.h>
#include <qscrollbar.h>
#include "roivertcore.h"

namespace
{
    bool clickOutOfBounds(const QPointF clickpos, const QSize imgsize)
    {
        return clickpos.x() < 0 || clickpos.x() > static_cast<qreal>(imgsize.width()) - 1 || clickpos.y() < 0 || clickpos.y() > static_cast<qreal>(imgsize.height()) - 1;
    }
    void clickToBounds(QPointF &clickpos, const QSize imgsize)
    {
        clickpos.setX(qMax(0., qMin(clickpos.x(), static_cast<qreal>(imgsize.width()) - 1)));
        clickpos.setY(qMax(0., qMin(clickpos.y(), static_cast<qreal>(imgsize.height()) - 1)));
    }
} // namespace
ImageROIViewer::ImageROIViewer(QWidget *parent)
    : QGraphicsView(parent)
{
    scene = new QGraphicsScene(parent);
    pixitem = new QGraphicsPixmapItem();
    pixitem->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    scene->addItem(pixitem);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setScene(scene);
    setBackgroundBrush(Qt::black);

    this->setParent(parent);
    setEnabled(false);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

// Image:
void ImageROIViewer::setImage(const QImage image)
{
    if (image.size().isEmpty())
    {
        return;
    }
    pixitem->setPixmap(QPixmap::fromImage(image));
    const QSize oldSize = img.size();
    const QSize newSize = image.size();
    img = image;
    if (oldSize != newSize)
    {
        scene->setSceneRect(pixitem->boundingRect());
        fitInView(pixitem, Qt::KeepAspectRatio);
        createROIMap();
    }
}
QSize ImageROIViewer::getImageSize()
{
    return img.size();
}

// ROI:
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
    setSelectedROI(rois.size());
}
void ImageROIViewer::deleteROI(size_t roiind)
{
    if (roiind > 0 && roiind <= rois.size())
    {
        // TODO: delete the roi:
        // if i make the base clase desrtuctor virtual, will it do virtual dispatch and delete the right thing?
        // try this...
        roi_rect *a = dynamic_cast<roi_rect *>(rois[roiind - 1]);
        roi_ellipse *b = dynamic_cast<roi_ellipse *>(rois[roiind - 1]);
        roi_polygon *c = dynamic_cast<roi_polygon *>(rois[roiind - 1]);
        if (a)
        {
            delete (a);
        }
        if (b)
        {
            delete (b);
        }
        if (c)
        {
            delete (c);
        }

        // set selroi to 0, but don't call setSelectedROI
        selroi = 0;

        // remove from vector
        rois.erase(rois.begin() + roiind - 1);

        // This emit will update the traceviewer
        emit roiDeleted(roiind);

        // Update the selection map
        createROIMap();
    }
}
void ImageROIViewer::setSelectedROI(size_t val)
{
    // This sets the selected roi, (1 indexed)
    const size_t prevind = selroi;
    const size_t newind = val;

    if (selroi > 0 && selroi<=rois.size())
    {
        rois[selroi - 1]->setColor(unselectedColor);
    }
    selroi = val;
    if (selroi > 0 && selroi <= rois.size())
    {
        rois[selroi - 1]->setColor(selectedColor);
    }
    // todo: emit something (with old and new)
    emit roiSelectionChange(prevind, newind);
}

// ROI Map:
void ImageROIViewer::createROIMap()
{
    roimap = cv::Mat(img.size().height(), img.size().width(), CV_16U);
    roimap = 0;

    for (size_t i = 0; i < rois.size(); i++)
    {
        updateROIMap(i + 1);
    }
}
void ImageROIViewer::updateROIMap(size_t roiind)
{
    if (roiind > 0)
    {
        const QRect bb(rois[roiind - 1]->getBB());
        const cv::Rect cvbb(ROIVert::QRect2CVRect(bb));
        cv::Mat boundedROIImage = roimap(cvbb);
        cv::Mat mask(rois[roiind - 1]->getMask());
        boundedROIImage.setTo(roiind, mask);
    }
}

// Mouse and resize:
void ImageROIViewer::resizeEvent(QResizeEvent *event)
{
    if (!verticalScrollBar()->isVisible() && !horizontalScrollBar()->isVisible())
    {
        fitInView(pixitem, Qt::KeepAspectRatio);
    }
}
void ImageROIViewer::mousePressEvent(QMouseEvent *event)
{
    // filter out right clicks:
    if (event->button() != Qt::LeftButton)
    {
        return; // skip right clicks for now
    }

    // pass ctrl+ clicks through to pan/zoom
    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
        setDragMode(QGraphicsView::ScrollHandDrag);
        QGraphicsView::mousePressEvent(event);
        return;
    }

    const QPointF clickpos = mapToScene(event->pos());

    // filter out clicks that are out of bounds
    if (clickOutOfBounds(clickpos, getImageSize()))
    {
        return;
    }

    // ROI add mode:
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
            // Polygon in progress
            QVector<QPoint> pt = rois[mousestatus.ActiveROI - 1]->getVertices();
            pt.push_back(clickpos.toPoint());
            mousestatus.ActiveVert++;
            rois[mousestatus.ActiveROI - 1]->setVertices(pt);
        }
    }
    else if (mousestatus.mode == ROIVert::SELROI)
    {
        setSelectedROI(roimap.at<unsigned short int>(static_cast<int>(clickpos.y()), static_cast<int>(clickpos.x())));
    }
}
void ImageROIViewer::mouseMoveEvent(QMouseEvent *event)
{
    // Look for mouse moves where we're in progress adding a new ROI
    if (mousestatus.ActiveROI == 0 || mousestatus.ActiveVert == 0)
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    };

    // clamp mouse location to bounds
    QPointF clickpos = mapToScene(event->pos());
    clickToBounds(clickpos, getImageSize());

    const size_t ROIind = mousestatus.ActiveROI - 1;
    const size_t VERTind = mousestatus.ActiveVert - 1;

    // Set vertex
    QVector<QPoint> verts = rois[ROIind]->getVertices();
    verts[VERTind] = clickpos.toPoint();
    rois[ROIind]->setVertices(verts);
    QGraphicsView::mouseMoveEvent(event);
}
void ImageROIViewer::mouseReleaseEvent(QMouseEvent *event)
{
    // Disable drag on release
    setDragMode(QGraphicsView::NoDrag);
    if (mousestatus.ActiveROI == 0 || mousestatus.ActiveVert == 0){return;};

    // if we were adding a square or ellipse, complete it
    if (mousestatus.mode == ROIVert::ADDROI && roishape != ROIVert::POLYGON && mousestatus.ActiveROI != 0)
    {
        emit roiEdited(mousestatus.ActiveROI);
        updateROIMap(mousestatus.ActiveROI);
        mousestatus.ActiveROI = 0;
        mousestatus.ActiveVert = 0;
    }
}
void ImageROIViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Filter out right clicks
    if (event->button() != Qt::LeftButton){return;}

    // if we were adding a polygon, complete it...
    if (mousestatus.mode == ROIVert::ADDROI && mousestatus.ActiveROI > 0 && mousestatus.ActiveVert > 0 && roishape == ROIVert::POLYGON)
    {
        // Special case for polygons, we finish it with a double click.
        // That means popping off the last added vertex (from the first click of the double):
        QVector<QPoint> pt = rois[mousestatus.ActiveROI - 1]->getVertices();
        pt.pop_back();
        pt.pop_back();
        rois[mousestatus.ActiveROI - 1]->setVertices(pt);

        emit roiEdited(mousestatus.ActiveROI);
        updateROIMap(mousestatus.ActiveROI);
        mousestatus.ActiveROI = 0;
        mousestatus.ActiveVert = 0;
    }
}
void ImageROIViewer::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) && mousestatus.mode == ROIVert::ADDROI && mousestatus.ActiveROI > 0 && mousestatus.ActiveVert > 0)
    {
        // Special case for polygons, we finish it with an enter:
        QVector<QPoint> pt = rois[mousestatus.ActiveROI - 1]->getVertices();
        pt.pop_back();
        rois[mousestatus.ActiveROI - 1]->setVertices(pt);

        updateROIMap(mousestatus.ActiveROI);
        emit roiEdited(mousestatus.ActiveROI);
        mousestatus.ActiveROI = 0;
        mousestatus.ActiveVert = 0;
    }
    else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_4)
    {
        emit toolfromkey(event->key());
    }
    else if (event->key() >= Qt::Key_5 && event->key() <= Qt::Key_9)
    {
        emit toolfromkey(event->key() + 1); //+1 for seperator
    }
    else if (event->key() == Qt::Key_Delete)
    {
        deleteROI(selroi);
    }
}
void ImageROIViewer::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
        QGraphicsView::wheelEvent(event);
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

// ROI import/export
QVector<QPair<ROIVert::ROISHAPE, QVector<QPoint>>> ImageROIViewer::getAllROIs()
{
    QVector<QPair<ROIVert::ROISHAPE, QVector<QPoint>>> ret;

    ret.reserve(rois.size());

    for each (roi* r in rois)
    {
        QPair<ROIVert::ROISHAPE, QVector<QPoint>> thisret;
        if (dynamic_cast<roi_rect*>(r)) { thisret.first = ROIVert::ROISHAPE::RECTANGLE; }
        if (dynamic_cast<roi_ellipse*>(r)) { thisret.first = ROIVert::ROISHAPE::ELLIPSE; }
        if (dynamic_cast<roi_polygon*>(r)) { thisret.first = ROIVert::ROISHAPE::POLYGON; }
        ret.push_back(thisret);
        thisret.second = r->getVertices();
    }
    return ret;
}
void ImageROIViewer::importROIs(const std::vector<roi *> &rois_in)
{
    for each(roi * r in rois_in)
    {
        // I think this should: figure out the type and cast, then copy (by calling new with dereference), then move the new pointer into the vector with emplace.
        const roi_rect *a = dynamic_cast<roi_rect *>(r);
        const roi_ellipse *b = dynamic_cast<roi_ellipse *>(r);
        const roi_polygon *c = dynamic_cast<roi_polygon *>(r);

        if (a){rois.emplace_back(new roi_rect(*a));}
        else if (b){rois.emplace_back(new roi_ellipse(*b));}
        else if (c){rois.emplace_back(new roi_polygon(*c));}

        rois.back()->setScene(scene);

        // update charts:
        emit roiEdited(rois.size());

        // select on the way in
        setSelectedROI(rois.size());
    }

    // update map
    createROIMap();
}

// Color Selection:
void ImageROIViewer::setSelectedColor(QColor clr)
{
    selectedColor = clr;
    if (selroi > 0)
    {
        rois[selroi - 1]->setColor(clr);
    }
}
void ImageROIViewer::setUnselectedColor(QColor clr)
{
    unselectedColor = clr;
    for (int i = 0; i < rois.size(); i++)
    {
        if (i != selroi - 1){rois[i]->setColor(clr);}
    }
}

// Passthrough getters
roi *ImageROIViewer::getRoi(size_t ind) { return rois[ind]; }
const size_t ImageROIViewer::getNROIs() { return rois.size(); }

// Zoom behavior...
Graphics_view_zoom::Graphics_view_zoom(QGraphicsView *view)
    : QObject(view), _view(view)
{
    _view->viewport()->installEventFilter(this);
    _view->setMouseTracking(true);
    _modifiers = Qt::ControlModifier;
    _zoom_factor_base = 1.0015;
}
void Graphics_view_zoom::gentle_zoom(double factor)
{
    if (!(_view->horizontalScrollBar()->isVisible() || _view->verticalScrollBar()->isVisible()) && factor < 1)
    {
        return;
    }

    _view->scale(factor, factor);
    _view->centerOn(target_scene_pos);
    const QPointF delta_viewport_pos = target_viewport_pos - QPointF(_view->viewport()->width() / 2.0,
                                                               _view->viewport()->height() / 2.0);
    const QPointF viewport_center = _view->mapFromScene(target_scene_pos) - delta_viewport_pos;
    _view->centerOn(_view->mapToScene(viewport_center.toPoint()));
    emit zoomed();
}
void Graphics_view_zoom::set_modifiers(Qt::KeyboardModifiers modifiers)
{
    _modifiers = modifiers;
}
void Graphics_view_zoom::set_zoom_factor_base(double value)
{
    _zoom_factor_base = value;
}
bool Graphics_view_zoom::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        const QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        const QPointF delta = target_viewport_pos - mouse_event->pos();
        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            target_viewport_pos = mouse_event->pos();
            target_scene_pos = _view->mapToScene(mouse_event->pos());
        }
    }
    else if (event->type() == QEvent::Wheel)
    {
        const QWheelEvent *wheel_event = static_cast<QWheelEvent *>(event);
        if (QApplication::keyboardModifiers() == _modifiers)
        {
            if (wheel_event->orientation() == Qt::Vertical)
            {
                const double angle = wheel_event->angleDelta().y();
                const double factor = qPow(_zoom_factor_base, angle);
                gentle_zoom(factor);
                return true;
            }
        }
    }
    return false;
}
