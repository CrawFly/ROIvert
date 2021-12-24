#pragma once
#include <QObject>
#include "ROIVertEnums.h"

class ROIs;
class QGraphicsItem;
class QMouseEvent;
class TraceViewWidget;
class TraceChartWidget;
class TraceChartSeries;
class ImageView;

class ROIController : public QObject {
    Q_OBJECT

public:
    ROIController(ROIs*, TraceViewWidget*, ImageView*);
    ~ROIController();
    
    void setSelected(std::vector<size_t>);
    std::vector<size_t> getSelected() const noexcept;

public slots:
    void mousePress(QList<QGraphicsItem*>, const QPointF&, QMouseEvent*);
    void keyPress(int, Qt::KeyboardModifiers);
    void chartClick(TraceChartWidget*, std::vector<TraceChartSeries*>, Qt::KeyboardModifiers);
    void setROIShape(ROIVert::SHAPE) noexcept;
    void imageSizeUpdate(QSize);
    void roiEdit(ROIVert::SHAPE, QRect, std::vector<QPoint>);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};