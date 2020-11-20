#include "roi.h"

// roi rect
roi_rect::roi_rect(QGraphicsScene *scene)
{
    thisroi = new QGraphicsRectItem();
    thisroi->setPen(getPen());
    scene->addItem(thisroi);
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

// roi ellipse
roi_ellipse::roi_ellipse(QGraphicsScene *scene)
{
    thisroi = new QGraphicsEllipseItem();
    thisroi->setPen(getPen());
    scene->addItem(thisroi);
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

// roi polygon
roi_polygon::roi_polygon(QGraphicsScene *scene)
{
    thisroi = new QGraphicsPolygonItem();
    thisroi->setPen(getPen());
    scene->addItem(thisroi);
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
        //thisroi->setVertices(verts);
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
