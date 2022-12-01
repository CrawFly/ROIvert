#pragma once
#include <QGraphicsItem>
#include "ROIShape.h"
class ROISelector : public QGraphicsItem
{
public:
    ROISelector(ROIShape* par);
    ~ROISelector();
    void setVertices(const QVector<QPoint>);
    void setBoundingRect(QRectF r);
    void setColor(const QColor clr);
    void setSize(double sz);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    enum { Type = UserType + 2 };
    int type() const override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
