#include "roi.h"

// roi rect
roi_rect::roi_rect(QGraphicsScene *scene)
{
    thisroi = new QGraphicsRectItem();
    thisroi->setPen(getPen());
    if (scene) {
        scene->addItem(thisroi);
    }
}
roi_rect::~roi_rect()
{
    if (thisroi)
    {
        delete thisroi;
    }
}
void roi_rect::setVertices(const QVector<QPoint> &verts)
{
    if (!verts.empty())
    {
        QRect rect;
        rect.setCoords(verts[0].x(), verts[0].y(), verts[0].x(), verts[0].y());
        for (size_t i = 1; i < verts.size(); i++)
        {
            rect.setLeft(qMin(rect.left(), verts[i].x()));
            rect.setRight(qMax(rect.right(), verts[i].x()));
            rect.setTop(qMin(rect.top(), verts[i].y()));
            rect.setBottom(qMax(rect.bottom(), verts[i].y()));
        }
        thisroi->setRect(rect);
        vertices = verts;
    }
}
QRect roi_rect::getBB()
{
    return thisroi->rect().toRect();
}

cv::Mat roi_rect::getMask() {
    QRect bb = getBB();

    int w = bb.width();
    int h = bb.height();
    cv::Size sz(w, h);
    cv::Mat mask(sz, CV_8U);
    mask = 255;
    return mask;
}

void roi_rect::setScene(QGraphicsScene* scene) {
    scene->addItem(thisroi);
}

// roi ellipse
roi_ellipse::roi_ellipse(QGraphicsScene *scene)
{
    thisroi = new QGraphicsEllipseItem();
    thisroi->setPen(getPen());
    if(scene){
        scene->addItem(thisroi);
    }
}
roi_ellipse::~roi_ellipse()
{
    if (thisroi)
    {
        delete thisroi;
    }
}
void roi_ellipse::setVertices(const QVector<QPoint> &verts)
{
    if (!verts.empty())
    {
        QRect rect;
        rect.setCoords(verts[0].x(), verts[0].y(), verts[0].x(), verts[0].y());
        for (size_t i = 1; i < verts.size(); i++)
        {
            rect.setLeft(qMin(rect.left(), verts[i].x()));
            rect.setRight(qMax(rect.right(), verts[i].x()));
            rect.setTop(qMin(rect.top(), verts[i].y()));
            rect.setBottom(qMax(rect.bottom(), verts[i].y()));
        }
        thisroi->setRect(rect);
        vertices = verts;
    }
}
QRect roi_ellipse::getBB()
{
    return thisroi->rect().toRect();
}
cv::Mat roi_ellipse::getMask() {
    QRect bb = getBB();

    int w = bb.width();
    int h = bb.height();
    cv::Size sz(w, h);
    cv::Mat mask(sz, CV_8U);
    mask = 0;
    cv::ellipse(mask, cv::Point(w / 2., h / 2.), cv::Size(w / 2., h / 2.), 0., 0., 360., cv::Scalar(255), cv::FILLED);
    return mask;
}


void roi_ellipse::setScene(QGraphicsScene* scene) {
    scene->addItem(thisroi);
}


// roi polygon
roi_polygon::roi_polygon(QGraphicsScene *scene)
{
    thisroi = new QGraphicsPolygonItem();
    thisroi->setPen(getPen());
    if (scene) {
        scene->addItem(thisroi);
    }
}
roi_polygon::~roi_polygon()
{
    if (thisroi)
    {
        delete thisroi;
    }
}

void roi_polygon::setVertices(const QVector<QPoint> &verts)
{
    if (!verts.empty())
    {
        thisroi->setPolygon(QPolygonF(verts));
        vertices = verts;
    }
}
QRect roi_polygon::getBB()
{
    // Note: this impl would work for all types, could put in the base...
    QRect out;
    // can always just iterate over all vertices:
    if (!vertices.isEmpty())
    {
        out.setCoords(vertices[0].x(), vertices[0].y(), vertices[0].x(), vertices[0].y());
        for (size_t i = 0; i < vertices.size(); i++)
        {
            out.setLeft(qMin(out.left(), vertices[i].x()));
            out.setRight(qMax(out.right(), vertices[i].x()));
            out.setTop(qMin(out.top(), vertices[i].y()));
            out.setBottom(qMax(out.bottom(), vertices[i].y()));
        }
    }
    return out;
}
cv::Mat roi_polygon::getMask() {
    QRect bb = getBB();

    int w = bb.width();
    int h = bb.height();
    cv::Size sz(w, h);
    cv::Mat mask(sz, CV_8U);
    mask = 0;

    float left = std::numeric_limits<float>::infinity();
    float top = std::numeric_limits<float>::infinity();
    foreach(QPoint vertex, vertices) {
        left = std::min((float)vertex.x(), left);
        top = std::min((float)vertex.y(), top);
    }

    // make points from qpoints?
    std::vector<cv::Point> cVertices;
    foreach(QPoint vertex, vertices)
    {
        cVertices.push_back(cv::Point(vertex.x() - left, vertex.y() - top));
    }
    cv::fillPoly(mask, cVertices, cv::Scalar(255));

    return mask;
}
void roi_polygon::setScene(QGraphicsScene* scene) {
    scene->addItem(thisroi);
}