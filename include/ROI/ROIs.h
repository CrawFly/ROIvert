#pragma once
#include <QObject>
#include "ImageView.h"
#include "TraceView.h"
#include "videodata.h"
#include "ROIVertEnums.h"
#include "ROIStyle.h"
#include "ChartStyle.h"

class ROIShape;

class TraceChartSeries;

class ROIs : public QObject
{
    Q_OBJECT

public:
    ROIs(ImageView*, TraceView*, VideoData*);
    ~ROIs();

    std::vector<size_t> getSelected() const noexcept;
    ROIStyle* getROIStyle(size_t ind) const noexcept;
    ROIStyle* getCoreROIStyle() const noexcept;

    ChartStyle* getLineChartStyle(size_t ind) const noexcept;
    void updateLineChartStyle(size_t ind);
    
    ChartStyle* getRidgeChartStyle(size_t ind) const noexcept;
    void updateRidgeChartStyle(size_t ind);

    void setColorBySelect(bool yesno = true);
    void updateROITraces();

    size_t getNROIs() const noexcept;
    void deleteAllROIs();
    std::vector<std::vector<float>> getTraces(std::vector<size_t> inds) const;
    void exportLineChartImages(std::vector<size_t> inds, QString basename, int width, int height, int quality) const;
    
    void setMatchYAxes(bool);

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

signals:
    void selectionChanged(std::vector<size_t> inds);

public slots:
    void mousePress(QList<QGraphicsItem*>, const QPointF&, QMouseEvent*);
    void keyPress(int, Qt::KeyboardModifiers);
    void imageSizeUpdate(QSize);
    void setROIShape(ROIVert::SHAPE) noexcept;
    void roiEdit(ROIVert::SHAPE, QRect, std::vector<QPoint>);
    void chartClick(TraceChartWidget*, std::vector<TraceChartSeries*>, Qt::KeyboardModifiers);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};