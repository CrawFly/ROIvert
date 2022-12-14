#include "ROI/ROIShape.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QGraphicsView>
#include <QJsonObject>
#include <QJsonArray>
#include <QMouseEvent>
#include <QPen>

#include "ROI/ROISelector.h"
#include "ROI/ROIStyle.h"

static void clamp_point_to_rect(QPoint& pt, QRect rect)
{
    pt.setX(std::clamp(pt.x(), rect.left(), rect.width()));
    pt.setY(std::clamp(pt.y(), rect.top(), rect.height()));
}

struct ROIShape::pimpl
{
    void init(ROIVert::SHAPE shape, ROIShape* parent, std::shared_ptr<ROIStyle> style)
    {
        switch (shape)
        {
        case ROIVert::SHAPE::RECTANGLE:
        {
            shapeItem = new QGraphicsRectItem(parent);
        }
        break;
        case ROIVert::SHAPE::ELLIPSE:
        {
            shapeItem = new QGraphicsEllipseItem(parent);
            break;
        }
        case ROIVert::SHAPE::POLYGON:
        {
            shapeItem = new QGraphicsPolygonItem(parent);
            break;
        }
        default:
            break;
        }
        par = parent;
        sel = new ROISelector(parent);
        shptype = shape;
        roistyle = style;

        updateStyle();
    }
    ROIShape* par;

    std::shared_ptr<ROIStyle> roistyle;

    QRectF bb;             // todo: rename to distinguish from roi bounding box
    int EditingVertex{ -1 }; // current vertex being edited, -1 for none

    QRect getBoundingBox() const noexcept
    {
        if (!vertices.empty())
        {
            QRect r(vertices[0], QSize(0, 0));
            for (auto& v : vertices)
            {
                r.setLeft(std::min(r.left(), v.x()));
                r.setRight(std::max(r.right(), v.x()));
                r.setTop(std::min(r.top(), v.y()));
                r.setBottom(std::max(r.bottom(), v.y()));
            }
            return r;
        }
        return QRect();
    }

    bool isPoly() const noexcept
    {
        return shptype == ROIVert::SHAPE::POLYGON;
    }
    void updateShape()
    {
        isPoly() ? updatePolyShape(shapeItem) : updateRectShape(shapeItem);
        sel->update();
        shapeItem->update();
    }
    void updateRectShape(QGraphicsItem* shp)
    {
        auto rShape = dynamic_cast<QGraphicsRectItem*>(shp);
        auto eShape = dynamic_cast<QGraphicsEllipseItem*>(shp);
        const QRect r{ getBoundingBox() };
        if (rShape)
        {
            rShape->setRect(r);
            rShape->setRect(r + QMargins(0, 0, -1, -1));
        }
        else if (eShape)
        {
            eShape->setRect(r);
            eShape->setRect(r + QMargins(0, 0, -1, -1));
        }
        sel->setVertices({ r.topLeft(), r.bottomLeft(), r.topRight(), r.bottomRight() });
    }
    void updatePolyShape(QGraphicsItem* shp)
    {
        auto poly = dynamic_cast<QGraphicsPolygonItem*>(shp);

        // ??? for some reason QPolygon::fromStdVector crashes...
        //QPolygon qvertices = QPolygon::fromStdVector(vertices);
        QPolygon qvertices;
        qvertices.reserve(vertices.size());
        for (auto& v : vertices)
        {
            qvertices.push_back(v);
        }

        if (poly)
        {
            poly->setPolygon(qvertices);
        }
        sel->setVertices(qvertices);
    }
    void editVertex(QPoint clickpos)
    {
        if (isPoly())
        {
            std::vector<int> d(vertices.size());
            std::transform(vertices.begin(), vertices.end(), d.begin(), [&](QPoint pt) -> int
            { return (clickpos - pt).manhattanLength(); });
            auto it = std::min_element(d.begin(), d.end());
            EditingVertex = it - d.begin();
        }
        else
        {
            QRect rect = getBoundingBox();

            std::vector<QPoint> vertpos{ rect.topLeft(), rect.topRight(), rect.bottomRight(), rect.bottomLeft() };

            std::vector<int> d(4);
            std::transform(vertpos.begin(), vertpos.end(), d.begin(), [&](QPoint pt) -> int
            { return (clickpos - pt).manhattanLength(); });

            auto it = std::min_element(d.begin(), d.end());
            size_t minind = it - d.begin();

            vertices = { vertpos[minind], vertpos[(minind + 2) % 4] };
            EditingVertex = 0;
        }
    }
    void setVertices(std::vector<QPoint> v)
    {
        vertices = v;
        updateShape();
    }
    void setVertex(QPoint v, size_t ind)
    {
        if (ind < vertices.size())
        {
            vertices[ind] = v;
            updateShape();
        }
        else if (ind == vertices.size())
        {
            vertices.push_back(v);
            updateShape();
        }
    }
    std::vector<QPoint> getVertices()
    {
        return vertices;
    }
    void mouseRelease(QPoint clickpos, const double sceneScale)
    {
        if (EditingVertex > -1)
        {
            if (isPoly() && polyopen)
            {
                QPointF distToFirstVertex = (clickpos - getVertices()[0]) * sceneScale;
                distToFirstVertex.setX(qAbs(distToFirstVertex.x()));
                distToFirstVertex.setY(qAbs(distToFirstVertex.y()));

                if (distToFirstVertex.x() < selsize && distToFirstVertex.y() < selsize)
                {
                    vertices.pop_back();
                    setVertices(vertices);
                    EditingVertex = -1;
                    polyopen = false;
                }
                else
                {
                    setVertex(clickpos, ++EditingVertex);
                }
            }
            else
            {
                EditingVertex = -1;
            }
        }
    }
    void updateStyle()
    {
        // set pen, set brush, set color on sel, set selsize
        pen = roistyle->getPen();
        brush = roistyle->getBrush();
        selsize = roistyle->getSelectorSize();

        auto shp = dynamic_cast<QAbstractGraphicsShapeItem*>(shapeItem);
        if (shp)
        {
            shp->setPen(pen);
            shp->setBrush(brush);
        }
        if (sel)
        {
            sel->setSize(selsize);
            sel->setColor(pen.color());
            sel->update();
        }
    }
    QPainterPath getSelectShape() const
    {
        if (sel)
            return sel->shape();
        return QPainterPath();
    }
    void setSelectVisible(bool visible)
    {
        if (sel)
            sel->setVisible(visible);
    }
    bool isSelectVisible()
    {
        if (sel)
            return sel->isVisible();
        return false;
    }
    void setSelectBoundingRect(QRectF r)
    {
        if (sel)
        {
            bb = r;
            sel->setBoundingRect(r);
        }
    }

    ROIVert::SHAPE getShapeType() const noexcept
    {
        return shptype;
    }
    void setShapeType(ROIVert::SHAPE shape)
    {
        delete shapeItem;
        delete sel;
        init(shape, par, roistyle);
        setSelectBoundingRect(bb);
    }

private:
    std::vector<QPoint> vertices;
    QGraphicsItem* shapeItem = nullptr;
    ROISelector* sel = nullptr;
    ROIVert::SHAPE shptype{ ROIVert::SHAPE::RECTANGLE };
    bool polyopen = true; // Special for distinguishing polygon's inital drawing state from later editing state
    QPen pen;
    QBrush brush;
    double selsize = 20.;
};

ROIShape::ROIShape(QGraphicsScene* scene,
    ROIVert::SHAPE shp,
    QSize imgsize,
    std::shared_ptr<ROIStyle> style) : impl(std::make_unique<pimpl>())
{
    impl->init(shp, this, style);
    setBoundingRect(QRectF(0, 0, imgsize.width(), imgsize.height()));
    connect(style.get(), &ROIStyle::StyleChanged, this, &ROIShape::updateStyle);
    scene->addItem(this);
}
ROIShape::~ROIShape() = default;

void ROIShape::setVertices(std::vector<QPoint> vertices)
{
    impl->setVertices(vertices);
}

void ROIShape::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint is no-op, children are auto painted.
}
QRectF ROIShape::boundingRect() const noexcept
{
    return impl->bb;
}
void ROIShape::setEditingVertex(int VertexIndex) noexcept
{
    impl->EditingVertex = VertexIndex;
}

// these three are overridden in poly? or we do it all in this object and just use switches
void ROIShape::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (!isSelectVisible() || event->button() != Qt::LeftButton)
    {
        return;
    }

    if (impl->EditingVertex < 0)
    { //signals nothing is currently being edited
        // trigger moving of an existing vertex
        QPoint clickpos = mapToScene(event->pos()).toPoint();
        impl->editVertex(clickpos);
    }
}

void ROIShape::doPress(QPoint pos)
{
    if (isSelectVisible() && impl->EditingVertex < 0)
    {
        impl->editVertex(pos);
        grabMouse();
    }
}

void ROIShape::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    // this means that the selector is moving!
    if (impl->EditingVertex > -1)
    {
        QPoint clickpos = mapToScene(event->pos()).toPoint();
        clamp_point_to_rect(clickpos, impl->bb.toRect());
        impl->setVertex(clickpos, impl->EditingVertex);
    }
}

void ROIShape::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    QPoint clickpos = mapToScene(event->pos()).toPoint();
    clamp_point_to_rect(clickpos, impl->bb.toRect());

    const double sceneScale = scene()->views()[0]->transform().m11();
    impl->mouseRelease(clickpos, sceneScale);
    if (impl->EditingVertex < 0)
    {
        ungrabMouse();
        emit roiEdited(getShapeType(), getTightBoundingBox(), getVertices());
    }
}

QPainterPath ROIShape::shape() const
{
    return impl->getSelectShape();
}

int ROIShape::type() const noexcept
{
    return Type;
}

void ROIShape::setSelectVisible(bool visible)
{
    impl->setSelectVisible(visible);
}

void ROIShape::setBoundingRect(QRectF r)
{
    impl->setSelectBoundingRect(r);
}

// Simple passthrough-impl getters:
std::vector<QPoint> ROIShape::getVertices() const noexcept { return impl->getVertices(); };
int ROIShape::getEditingVertex() const noexcept { return impl->EditingVertex; }

bool ROIShape::isSelectVisible() const noexcept
{
    return impl->isSelectVisible();
}

void ROIShape::updateStyle()
{
    impl->updateStyle();
}

ROIVert::SHAPE ROIShape::getShapeType() const noexcept
{
    return impl->getShapeType();
}

QRect ROIShape::getTightBoundingBox() const noexcept
{
    return impl->getBoundingBox();
}

void ROIShape::read(const QJsonObject & json, int pixelsubset)
{
    impl->setShapeType(static_cast<ROIVert::SHAPE>(json["type"].toInt()));
    QJsonArray jverts = json["verts"].toArray();
    std::vector<QPoint> vertices;
    for (const auto& vert : jverts)
    {
        QPoint pt{ vert.toArray()[0].toInt(), vert.toArray()[1].toInt() };
        vertices.push_back(pt / pixelsubset);
    }
    impl->setVertices(vertices);
    impl->setSelectVisible(false);
    emit roiEdited(getShapeType(), getTightBoundingBox(), getVertices());
}
void ROIShape::write(QJsonObject & json, int pixelsubset) const
{
    json["type"] = static_cast<int>(impl->getShapeType());
    QJsonArray jverts;
    for (auto& pt : impl->getVertices())
    {
        jverts.append(QJsonArray({ pt.x() * pixelsubset, pt.y() * pixelsubset }));
    }
    json["verts"] = jverts;
}