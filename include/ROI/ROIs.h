#pragma once
#include <QObject>
class ImageView;
class TraceViewWidget;
class VideoData;
class ROIStyle;
class ChartStyle;
#include "ROIVertEnums.h"

class ROIShape;
struct ROI;

class TraceChartWidget;
class TraceChartSeries;

class ROIs : public QObject
{
    Q_OBJECT

public:
    ROIs(ImageView*, TraceViewWidget*, VideoData*);
    ~ROIs();

    void pushROI(QPoint pos, ROIVert::SHAPE shp);
    void setROIShape(ROIVert::SHAPE);
    void update();

    void setSelected(std::vector<size_t>);
    std::vector<size_t> getSelected() const noexcept;

    ROIStyle* getROIStyle(size_t ind) const noexcept;
    ROIStyle* getCoreROIStyle() const noexcept;

    ChartStyle* getLineChartStyle(size_t ind) const noexcept;
    void updateLineChartStyle(size_t ind);
   
    ChartStyle* getRidgeChartStyle(size_t ind) const noexcept;
    void updateRidgeChartStyle(size_t ind);

    void setColorBySelect(bool yesno = true);

    void updateROITraces();

    ROI* getROI(size_t ind) const;
    size_t getNROIs() const noexcept;

    void deleteROIs(std::vector<size_t> inds);
    void deleteAllROIs();

    std::vector<std::vector<float>> getTraces(std::vector<size_t> inds) const;
    void exportLineChartImages(std::vector<size_t> inds, QString basename, int width, int height, int quality) const;
    
    void setMatchYAxes(bool);
    bool getMatchYAxes() const noexcept;
    
    int getIndex(const ROIShape* r) const;
    int getIndex(const TraceChartWidget* chart) const;
    int getIndex(const TraceChartSeries* series) const;

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

signals:
    void selectionChanged(std::vector<size_t> inds);


private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
