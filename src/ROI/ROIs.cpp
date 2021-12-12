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
#include "ROI/ROI.h"
#include "ROI/ROIController.h"
#include "ROI/ROISelector.h"
#include "ROI/ROIStyle.h"
#include "widgets/TraceChartWidget.h"

struct ROIs::pimpl
{
    QGraphicsScene* scene{ nullptr };
    TraceViewWidget* traceview{ nullptr };
    VideoData* videodata{ nullptr };
    ImageView* imageview{ nullptr };
    
    std::unique_ptr<ROIController> roicontroller;
    ROIs *par{nullptr};

    std::vector<std::unique_ptr<ROI>> rois;
    ROIPalette pal;
    ROIStyle coreStyle;
    std::vector<size_t> selectedROIs;
    bool matchyaxes{false};

    void setSelectedROIs(std::vector<size_t> inds)
    {
        for (auto &ind : selectedROIs)
        {
            if (ind < rois.size())
            {
                rois.at(ind)->graphicsShape->setSelectVisible(false);
                rois.at(ind)->roistyle->setSelected(false);
                rois.at(ind)->Trace->getLineSeries()->setHighlighted(false);
                rois.at(ind)->Trace->getRidgeSeries()->setHighlighted(false);
                rois.at(ind)->Trace->getTraceChart()->update();
            }
        }
        selectedROIs = inds;
        for (auto &ind : selectedROIs)
        {
            if (ind < rois.size())
            {
                rois.at(ind)->graphicsShape->setSelectVisible(true);
                rois.at(ind)->roistyle->setSelected(true);
                rois.at(ind)->Trace->getLineSeries()->setHighlighted(true);
                rois.at(ind)->Trace->getRidgeSeries()->setHighlighted(true);
                rois.at(ind)->Trace->getTraceChart()->update();
            }
        }
        traceview->update();
        if (!selectedROIs.empty() && selectedROIs.back() < rois.size())
        {
            traceview->scrollToChart(rois.at(selectedROIs.back())->Trace->getTraceChart());
        }
        if (par)
        {
            emit par->selectionChanged(selectedROIs);
        }
    }
    std::vector<std::unique_ptr<ROI>>::iterator find(const ROIShape *r) noexcept
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
    std::vector<std::unique_ptr<ROI>>::iterator find(const TraceChartWidget *chart) noexcept
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
    std::vector<std::unique_ptr<ROI>>::iterator find(const TraceChartSeries *series) noexcept
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
    
    void pushROI(QPoint pos, ROIVert::SHAPE shp) {
        ROIStyle rs = coreStyle;
        rs.setColor(pal.getPaletteColor(rois.size()));

        rois.push_back(std::make_unique<ROI>(scene, traceview, videodata, shp, imageview->getImageSize(), rs));
        
        auto &gObj = rois.back()->graphicsShape;
        if (!pos.isNull())
        {
            // pos should only be NULL for an import push
            gObj->setVertices({pos, pos});
            gObj->setEditingVertex(1);
            gObj->grabMouse();
        }

    }

    void deleteROIs(std::vector<size_t> inds)
    {
        // Unselect:
        std::vector<size_t> selected = selectedROIs;
        std::sort(selected.begin(), selected.end());
        std::sort(inds.begin(), inds.end());
        std::vector<size_t> updatedSelected;
        std::set_difference(selected.begin(), selected.end(), inds.begin(), inds.end(),
                            std::inserter(updatedSelected, updatedSelected.begin()));
        setSelectedROIs(updatedSelected);

        for (auto &ind : inds)
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
        scene->update();
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
            for (auto &roi : rois)
            {
                roi->linechartstyle->setLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
            }
            updateYLimits();
        }
        else
        {
            for (auto &roi : rois)
            {
                roi->linechartstyle->setLimitStyle(ROIVert::LIMITSTYLE::AUTO);
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

            for (auto &roi : rois)
            {
                themin = std::min(themin, roi->Trace->getLineSeries()->getYMin());
                themax = std::max(themax, roi->Trace->getLineSeries()->getYMax());
            }
            for (auto &roi : rois)
            {
                roi->linechartstyle->setLimitStyle(ROIVert::LIMITSTYLE::MANAGED);
                roi->Trace->getTraceChart()->getYAxis()->setManualLimits(themin, themax);
                roi->Trace->update();
            }
        }
    }
};

ROIs::ROIs(ImageView *iView, TraceViewWidget *tView, VideoData *vData) : impl(std::make_unique<pimpl>())
{
    impl->imageview = iView;
    impl->scene = iView->scene();
    impl->traceview = tView;
    impl->videodata = vData;
    impl->roicontroller = std::make_unique<ROIController>(this, tView, iView);

    impl->par = this;

    connect(&tView->getRidgeChart(), &TraceChartWidget::chartClicked, impl->roicontroller.get(), &ROIController::chartClick);
    connect(iView, &ImageView::mousePressed, impl->roicontroller.get(), &ROIController::mousePress);
    connect(iView, &ImageView::keyPressed, impl->roicontroller.get(), &ROIController::keyPress);
    connect(iView, &ImageView::imageSizeUpdated, impl->roicontroller.get(), &ROIController::imageSizeUpdate);
    connect(tView, &TraceViewWidget::chartClicked, impl->roicontroller.get(), &ROIController::chartClick);
    connect(tView, &TraceViewWidget::keyPressed, impl->roicontroller.get(), &ROIController::keyPress);
}
ROIs::~ROIs() = default;


void ROIs::setSelected(std::vector<size_t> inds)
{
    impl->setSelectedROIs(inds);
};

std::vector<size_t> ROIs::getSelected() const noexcept
{
    return impl->selectedROIs;
};

ROIStyle *ROIs::getROIStyle(size_t ind) const noexcept
{
    if (ind > impl->rois.size())
    {
        return nullptr;
    }
    return impl->rois[ind]->roistyle.get();
}

void ROIs::setColorBySelect(bool yesno)
{
    impl->coreStyle.setColorBySelected(yesno);
    for (auto &r : impl->rois)
    {
        r->roistyle->setColorBySelected(yesno);
    }
}

void ROIs::updateROITraces()
{
    for (auto &r : impl->rois)
    {
        r->Trace->update();
    }
}

size_t ROIs::getNROIs() const noexcept { return impl->rois.size(); }

void ROIs::deleteROIs(std::vector<size_t> inds) {
    impl->deleteROIs(inds);
}

void ROIs::deleteAllROIs()
{
    std::vector<size_t> inds(impl->rois.size());
    std::iota(inds.begin(), inds.end(), 0);
    impl->deleteROIs(inds);
}

std::vector<std::vector<float>> ROIs::getTraces(std::vector<size_t> inds) const
{
    auto out = std::vector<std::vector<float>>();
    out.reserve(inds.size());

    for (auto &ind : inds)
    {
        if (ind < impl->rois.size())
        {
            out.push_back(impl->rois.at(ind)->Trace->getTrace());
        }
    }
    return out;
}

void ROIs::read(const QJsonObject &json)
{
    QJsonArray jrois = json["ROIs"].toArray();
    for (const auto &jroi : jrois)
    {
        impl->pushROI(QPoint(), ROIVert::SHAPE::RECTANGLE);
        impl->rois.back()->read(jroi.toObject());
    }
}
void ROIs::write(QJsonObject &json) const
{
    QJsonArray jrois;
    for (auto &r : impl->rois)
    {
        QJsonObject jroi;
        r->write(jroi);
        jrois.append(jroi);
    }
    json["ROIs"] = jrois;
}

void ROIs::exportLineChartImages(std::vector<size_t> inds, QString basename, int width, int height, int quality) const
{
    for (auto &ind : inds)
    {
        if (ind < impl->rois.size())
        {
            const QString filename(basename + "_" + QString::number(ind + 1) + ".png");
            impl->rois.at(ind)->Trace->getTraceChart()->saveAsImage(filename, width, height, quality);
        }
    }
}

ROIStyle *ROIs::getCoreROIStyle() const noexcept
{
    return &impl->coreStyle;
}

ChartStyle *ROIs::getLineChartStyle(size_t ind) const noexcept
{
    return impl->rois.at(ind)->linechartstyle.get();
}
void ROIs::updateLineChartStyle(size_t ind)
{
    impl->rois.at(ind)->Trace->getTraceChart()->updateStyle();
    impl->updateYLimits();
}

ChartStyle *ROIs::getRidgeChartStyle(size_t ind) const noexcept
{
    return impl->rois.at(ind)->ridgechartstyle.get();
}
void ROIs::updateRidgeChartStyle(size_t ind)
{
    impl->traceview->getRidgeChart().updateStyle();
}

void ROIs::setMatchYAxes(bool onoff)
{
    impl->setMatchYAxes(onoff);
}
bool ROIs::getMatchYAxes() const noexcept { return impl->matchyaxes; }

int ROIs::getIndex(const ROIShape *r) const
{
    const auto it = impl->find(r);
    return it == impl->rois.end() ? -1 : it - impl->rois.begin();
}
int ROIs::getIndex(const TraceChartWidget *chart) const
{
    const auto it = impl->find(chart);
    return it == impl->rois.end() ? -1 : it - impl->rois.begin();
} 
int ROIs::getIndex(const TraceChartSeries *series) const
{
    const auto it = impl->find(series);
    return it == impl->rois.end() ? -1 : it - impl->rois.begin();
}

ROI* ROIs::getROI(size_t ind) const {
    return impl->rois[ind].get();
}


void ROIs::pushROI(QPoint pos, ROIVert::SHAPE shp)
{
    impl->pushROI(pos, shp);
    auto &gObj = impl->rois.back()->graphicsShape;
    auto &tObj = impl->rois.back()->Trace;
    connect(gObj.get(), &ROIShape::roiEdited, tObj.get(), &ROITrace::updateTrace);
    connect(gObj.get(), &ROIShape::roiEdited, impl->roicontroller.get(), &ROIController::roiEdit);
    connect(tObj->getTraceChart(), &TraceChartWidget::chartClicked, impl->roicontroller.get(), &ROIController::chartClick);
}

void ROIs::update() {
    impl->scene->update();
    impl->updateYLimits();
}
    

void ROIs::setROIShape(ROIVert::SHAPE shp) {
    impl->roicontroller->setROIShape(shp);
}