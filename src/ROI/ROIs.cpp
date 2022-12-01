#include "ROI/ROIs.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QMouseEvent>

#include "VideoData.h"

#include "ChartStyle.h"
#include "dockwidgets/TraceViewWidget.h"
#include "ImageView.h"
#include "ROI/ROIController.h"
#include "ROI/ROISelector.h"
#include "ROI/ROIStyle.h"
#include "widgets/TraceChartWidget.h"

struct ROIs::pimpl
{
    TraceViewWidget* traceview{ nullptr };
    VideoData* videodata{ nullptr };
    ImageView* imageview{ nullptr };
    std::unique_ptr<ROIController> roicontroller;

    std::vector<std::unique_ptr<ROI>> rois;
    ROIPalette pal;
    ROIStyle coreStyle;
    bool matchyaxes{ false };

    std::vector<std::unique_ptr<ROI>>::iterator find(const ROIShape* r) noexcept
    {
        for (auto it = rois.begin(); it < rois.end(); ++it)
        {
            if ((*it)->graphicsShape.get() == r)
            {
                return it;
            }
        }
        return rois.end();
    }
    std::vector<std::unique_ptr<ROI>>::iterator find(const TraceChartWidget* chart) noexcept
    {
        for (auto it = rois.begin(); it < rois.end(); ++it)
        {
            if ((*it)->Trace->getTraceChart() == chart)
            {
                return it;
            }
        }
        return rois.end();
    }
    std::vector<std::unique_ptr<ROI>>::iterator find(const TraceChartSeries* series) noexcept
    {
        for (auto it = rois.begin(); it < rois.end(); ++it)
        {
            if ((*it)->Trace->getRidgeSeries() == series)
            {
                return it;
            }
        }
        return rois.end();
    }

    void pushROI(QPoint pos, ROIVert::SHAPE shp, bool isimport = false) {
        ROIStyle rs = coreStyle;
        rs.setColor(pal.getPaletteColor(rois.size()));

        rois.push_back(std::make_unique<ROI>(imageview->scene(), traceview, videodata, shp, imageview->getImageSize(), rs));

        auto& gObj = rois.back()->graphicsShape;
        if (!isimport)
        {
            gObj->setVertices({ pos, pos });
            gObj->setEditingVertex(1);
            gObj->grabMouse();
        }
    }

    void deleteROIs(std::vector<size_t> inds)
    {
        for (auto& ind : inds)
        {
            rois[ind] = nullptr;
        }

        // clean up the vector
        for (auto it = rois.begin(); it != rois.end();)
        {
            if (*it == nullptr)
            {
                it = rois.erase(it);
            }
            else
            {
                ++it;
            }
        }
        imageview->scene()->update();
        updateYLimits();
    }

    void setMatchYAxes(bool onoff)
    {
        if (matchyaxes == onoff)
        {
            return;
        }
        matchyaxes = onoff;

        if (onoff)
        {
            // matchy
            for (auto& roi : rois)
            {
                roi->linechartstyle->setYLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
            }
            updateYLimits();
        }
        else
        {
            for (auto& roi : rois)
            {
                roi->linechartstyle->setYLimitStyle(ROIVert::LIMITSTYLE::AUTO);
                roi->Trace->update();
            }
        }
    }
    void updateYLimits()
    {
        if (matchyaxes)
        {
            double themin = std::numeric_limits<double>::infinity();
            double themax = -std::numeric_limits<double>::infinity();

            for (auto& roi : rois)
            {
                themin = std::min(themin, roi->Trace->getLineSeries()->getYMin());
                themax = std::max(themax, roi->Trace->getLineSeries()->getYMax());
            }
            for (auto& roi : rois)
            {
                roi->linechartstyle->setYLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
                roi->Trace->getTraceChart()->getYAxis()->setManualLimits(themin, themax);
                roi->Trace->update();
            }
        }
    }
};

ROIs::ROIs(ImageView* iView, TraceViewWidget* tView, VideoData* vData) : impl(std::make_unique<pimpl>())
{
    impl->imageview = iView;
    impl->traceview = tView;
    impl->videodata = vData;
    impl->roicontroller = std::make_unique<ROIController>(this, tView, iView);

    connect(&tView->getRidgeChart(), &TraceChartWidget::chartClicked, impl->roicontroller.get(), &ROIController::chartClick);
    connect(iView, &ImageView::mousePressed, impl->roicontroller.get(), &ROIController::mousePress);
    connect(iView, &ImageView::keyPressed, impl->roicontroller.get(), &ROIController::keyPress);
    connect(iView, &ImageView::imageSizeUpdated, impl->roicontroller.get(), &ROIController::imageSizeUpdate);
    connect(tView, &TraceViewWidget::chartClicked, impl->roicontroller.get(), &ROIController::chartClick);
    connect(tView, &TraceViewWidget::keyPressed, impl->roicontroller.get(), &ROIController::keyPress);
}
ROIs::~ROIs() = default;

size_t ROIs::size() const noexcept { return impl->rois.size(); }
ROI& ROIs::operator[](std::size_t idx) {
    return *impl->rois[idx];
}
const ROI& ROIs::operator[](std::size_t idx) const {
    return *impl->rois[idx];
}
void ROIs::pushROI(QPoint pos, ROIVert::SHAPE shp)
{
    impl->pushROI(pos, shp, false);
    auto& gObj = impl->rois.back()->graphicsShape;
    auto& tObj = impl->rois.back()->Trace;
    connect(gObj.get(), &ROIShape::roiEdited, tObj.get(), &ROITrace::updateTrace);
    connect(gObj.get(), &ROIShape::roiEdited, impl->roicontroller.get(), &ROIController::roiEdit);
    connect(tObj->getTraceChart(), &TraceChartWidget::chartClicked, impl->roicontroller.get(), &ROIController::chartClick);
}

void ROIs::setSelected(std::vector<size_t> inds) {
    impl->roicontroller->setSelected(inds);
}

std::vector<size_t> ROIs::getSelected() const noexcept
{
    return impl->roicontroller->getSelected();
};

void ROIs::setColorBySelect(bool yesno)
{
    impl->coreStyle.setColorBySelected(yesno);
    for (auto& r : impl->rois)
    {
        r->roistyle->setColorBySelected(yesno);
    }
}

void ROIs::updateROITraces()
{
    for (auto& r : impl->rois)
    {
        r->Trace->update();
    }
}

void ROIs::deleteROIs(std::vector<size_t> inds) {
    impl->deleteROIs(inds);
}

void ROIs::deleteAllROIs()
{
    std::vector<size_t> inds(impl->rois.size());
    std::iota(inds.begin(), inds.end(), 0);
    impl->deleteROIs(inds);
}

void ROIs::read(const QJsonObject & json)
{
    QJsonArray jrois = json["ROIs"].toArray();
    for (const auto& jroi : jrois)
    {
        impl->pushROI(QPoint(), ROIVert::SHAPE::RECTANGLE, true);
        impl->rois.back()->read(jroi.toObject());
    }
}
void ROIs::write(QJsonObject & json) const
{
    QJsonArray jrois;
    for (auto& r : impl->rois)
    {
        QJsonObject jroi;
        r->write(jroi);
        jrois.append(jroi);
    }
    json["ROIs"] = jrois;
}

ROIStyle* ROIs::getCoreROIStyle() const noexcept
{
    return &impl->coreStyle;
}

void ROIs::setMatchYAxes(bool onoff)
{
    impl->setMatchYAxes(onoff);
}
bool ROIs::getMatchYAxes() const noexcept { return impl->matchyaxes; }

int ROIs::getIndex(const ROIShape * r) const
{
    const auto it = impl->find(r);
    return it == impl->rois.end() ? -1 : it - impl->rois.begin();
}
int ROIs::getIndex(const TraceChartWidget * chart) const
{
    const auto it = impl->find(chart);
    return it == impl->rois.end() ? -1 : it - impl->rois.begin();
}
int ROIs::getIndex(const TraceChartSeries * series) const
{
    const auto it = impl->find(series);
    return it == impl->rois.end() ? -1 : it - impl->rois.begin();
}

void ROIs::update() {
    impl->imageview->scene()->update();
    impl->updateYLimits();
}

void ROIs::setROIShape(ROIVert::SHAPE shp) {
    impl->roicontroller->setROIShape(shp);
}