#include "ZoomPan.h"

#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>

struct ZoomPan::pimpl
{
    QGraphicsView *view{nullptr};
    Qt::KeyboardModifiers mod = Qt::ControlModifier;
    double zoomfactor = 1.0015;
    bool zoomedMin()
    {
        return !(view->horizontalScrollBar()->isVisible() || view->verticalScrollBar()->isVisible());
    }

    const QPointF deltaViewportPos()
    {
        return QPointF(targetViewportPos - QPointF(view->viewport()->width() / 2., view->viewport()->height() / 2.));
    }
    const QPointF viewportCenter()
    {
        return QPointF(view->mapFromScene(targetScenePos) - deltaViewportPos());
    }

    void zoom(const double factor)
    {
        if (factor < 1 && zoomedMin())
            return;
        view->scale(factor, factor);
        view->centerOn(targetScenePos);
        view->centerOn(view->mapToScene(viewportCenter().toPoint()));
    }

    void moveEvent(const QMouseEvent *event)
    {

        const QPointF delta = targetViewportPos - event->pos();
        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            targetViewportPos = event->pos();
            targetScenePos = view->mapToScene(event->pos());
        }
        // manual implementation of drag, something interferes with dragMode
        if (view->dragMode() == QGraphicsView::ScrollHandDrag)
        {
            view->centerOn(view->mapToScene((viewportCenter() + delta).toPoint()));
        }
    }
    bool wheelEvent(const QWheelEvent *event)
    {
        if (QApplication::keyboardModifiers() == mod)
        {
            const double angle = event->angleDelta().y();
            const double factor = pow(zoomfactor, angle);
            zoom(factor);
            return true;
        }
        return false;
    }
    bool pressEvent(const QMouseEvent *event)
    {
        if (QApplication::keyboardModifiers() == mod)
        {
            view->setDragMode(QGraphicsView::ScrollHandDrag);
            return true;
        }
        return false;
    }

    QPointF targetScenePos, targetViewportPos;
};

ZoomPan::ZoomPan(QGraphicsView *view) : QObject(view), impl(std::make_unique<pimpl>())
{
    impl->view = view;
    impl->view->viewport()->installEventFilter(this);
    impl->view->setMouseTracking(true);
}
ZoomPan::~ZoomPan() { };

void ZoomPan::setZoomFactor(double value) noexcept { impl->zoomfactor = value; }
bool ZoomPan::eventFilter(QObject *object, QEvent *event)
{

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        return impl->pressEvent(dynamic_cast<QMouseEvent *>(event));
    case QEvent::MouseMove:
        impl->moveEvent(dynamic_cast<QMouseEvent *>(event));
        return false;
    case QEvent::Wheel:
        return impl->wheelEvent(dynamic_cast<QWheelEvent *>(event));
    case QEvent::MouseButtonRelease:
        impl->view->setDragMode(QGraphicsView::NoDrag);
        return false;
    default:
        break;
    }
    return false;
}
