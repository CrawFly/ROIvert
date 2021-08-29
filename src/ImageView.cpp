#include "ImageView.h"

#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QScrollBar>

#include "ZoomPan.h"

struct ImageView::pimpl
{
    QGraphicsPixmapItem *pix = new QGraphicsPixmapItem;
    QGraphicsScene *scene = new QGraphicsScene;

    QSize imgsize;
    std::unique_ptr<ZoomPan> zoomer;
};

ImageView::ImageView(QWidget *parent) : QGraphicsView(parent)
{
    setScene(impl->scene);
    setBackgroundBrush(Qt::black);
    impl->pix->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    impl->scene->addItem(impl->pix);
    impl->scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    impl->zoomer = std::make_unique<ZoomPan>(this);
}

ImageView::~ImageView() = default;

void ImageView::setImage(const QImage &image)
{
    impl->pix->setPixmap(QPixmap::fromImage(image));

    if (impl->imgsize != image.size())
    {
        impl->imgsize = image.size();
        impl->scene->setSceneRect(QRect(0, 0, impl->imgsize.width(), impl->imgsize.height()));

        fitInView(impl->pix, Qt::KeepAspectRatio);
        emit imageSizeUpdated(impl->imgsize);
    }
}

void ImageView::resizeEvent(QResizeEvent *event)
{
    if (!verticalScrollBar()->isVisible() && !horizontalScrollBar()->isVisible())
    {
        fitInView(impl->pix, Qt::KeepAspectRatio);
    }
}

void ImageView::mousePressEvent(QMouseEvent *event)
{
    // Instead of relying on Qt to dispatch events, mousePressEvent is
    // going to fire its signal and NOT call the base class event...
    //
    // ROIs will listen to this event and react accordingly...
    const QPointF clickpos = mapToScene(event->pos());
    emit mousePressed(items(event->pos()), clickpos, event);
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF clickpos = mapToScene(event->pos());
    emit mouseMoved(clickpos, sceneRect().contains(clickpos));
    QGraphicsView::mouseMoveEvent(event);
}
void ImageView::keyPressEvent(QKeyEvent *event)
{
    emit keyPressed(event->key(), event->modifiers());
}

QSize ImageView::getImageSize() const noexcept { return impl->imgsize; }