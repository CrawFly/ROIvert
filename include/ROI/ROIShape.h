#pragma once
#include <QGraphicsItem>
#include "ROIVertEnums.h"

class ROIStyle;


class ROIShape : public QGraphicsObject
{
    Q_OBJECT

public:
    ROIShape(QGraphicsScene* scene,
        ROIVert::SHAPE,
        QSize imgsize,
        const ROIStyle& style);

    ~ROIShape();
    
    void setVertices(std::vector<QPoint> vertices);
    std::vector<QPoint> getVertices() const noexcept;
    
    void setEditingVertex(int VertexIndex) noexcept;
    int getEditingVertex() const noexcept;

    void setSelectVisible(bool visible = true);
    bool isSelectVisible() const noexcept;

    void setBoundingRect(QRectF); // NOTE that this boundingrect is the one used for painting, NOT the tight rect
    void updateStyle(const ROIStyle& style);
    
    ROIVert::SHAPE getShapeType() const noexcept;
    QRect getTightBoundingBox() const noexcept;

    // Overrides to support painting, selection, cliping, etc.
    enum { Type = UserType + 1 };
    int type() const noexcept override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;
    QRectF boundingRect() const noexcept override;
    QPainterPath shape() const override;


    void doPress(QPoint pos);

signals:
    void roiEdited(ROIVert::SHAPE, QRect, std::vector<QPoint>);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};
