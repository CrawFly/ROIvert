#pragma once
#include <QObject>
class ImageView;
class TraceViewWidget;
class VideoData;
class ROIStyle;
class ChartStyle;
#include "ROIVertEnums.h"
#include "ROI/ROI.h"
class ROIShape;

class TraceChartWidget;
class TraceChartSeries;

class ROIs : public QObject
{
    Q_OBJECT

public:
    ROIs(ImageView*, TraceViewWidget*, VideoData*);
    ~ROIs();

    size_t size() const noexcept;
    ROI& operator[](std::size_t idx);
    const ROI& operator[](std::size_t idx) const;
    void pushROI(QPoint pos, ROIVert::SHAPE shp);

    void deleteROIs(std::vector<size_t> inds);
    void deleteAllROIs();

    void setROIShape(ROIVert::SHAPE);
    void update();

    int getIndex(const ROIShape* r) const;
    int getIndex(const TraceChartWidget* chart) const;
    int getIndex(const TraceChartSeries* series) const;

    void setSelected(std::vector<size_t>);
    std::vector<size_t> getSelected() const noexcept;

    ROIStyle* getCoreROIStyle() const noexcept;
    void setColorBySelect(bool yesno = true);

    void setMatchYAxes(bool);
    bool getMatchYAxes() const noexcept;

    void updateROITraces();

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;

signals:
    void selectionChanged(std::vector<size_t> inds);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
