#pragma once
#include <QGraphicsItem>
#include "ROIVertEnums.h"

class ROIShape : public QGraphicsObject
{
    Q_OBJECT

public:
    ROIShape(QGraphicsScene* scene,
        ROIVert::SHAPE,
        QSize imgsize,
        double selsize, 
        QPen pen,
        QBrush brush);

    ~ROIShape();
    
    void setVertices(std::vector<QPoint> vertices);
    std::vector<QPoint> getVertices() const noexcept;
    
    void setEditingVertex(int VertexIndex) noexcept;
    int getEditingVertex() const noexcept;

    void setSelectVisible(bool visible = true);
    bool isSelectVisible() const noexcept;

    void setBoundingRect(QRectF);


    void updateStyle(QPen pen, QBrush brush, int selsize);
    

    // Overrides to support painting, selection, cliping, etc.
    enum { Type = UserType + 1 };
    int type() const noexcept override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;
    QRectF boundingRect() const noexcept override;
    QPainterPath shape() const override;


    void doPress(QPoint pos);

signals:
    void roiEdited(ROIShape *roi);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

