#include "ROI/ROIs.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QMouseEvent>

#include "ChartStyle.h"
#include "ROI/ROI.h"
#include "ROI/ROISelector.h"
#include "ROI/ROIStyle.h"
#include "widgets/TraceChartWidget.h"

struct ROIs::pimpl
{
    QGraphicsScene *scene{nullptr};
    TraceViewWidget *traceview{nullptr};
    VideoData *videodata{nullptr};
    ROIs *par{nullptr};

    std::vector<std::unique_ptr<ROI>> rois;
    ROIPalette pal;
    ROIStyle coreStyle;
    QSize imgsize;
    std::vector<size_t> selectedROIs;
    ROIVert::SHAPE mousemode{ROIVert::SHAPE::ELLIPSE};
    bool matchyaxes{false};

    void pushROI(QPoint pos)
    {
        ROIStyle rs = coreStyle;
        rs.setColor(pal.getPaletteColor(rois.size()));

        rois.push_back(std::make_unique<ROI>(scene, traceview, videodata, mousemode, imgsize, rs));

        auto &gObj = rois.back()->graphicsShape;
        auto &tObj = rois.back()->Trace;

        if (!pos.isNull())
        {
            // pos should only be NULL for an import push
            gObj->setVertices({pos, pos});
            gObj->setEditingVertex(1);
            gObj->grabMouse();
        }

        connect(gObj.get(), &ROIShape::roiEdited, tObj.get(), &ROITrace::updateTrace);
        connect(gObj.get(), &ROIShape::roiEdited, par, &ROIs::roiEdit);
        connect(tObj->getTraceChart(), &TraceChartWidget::chartClicked, par, &ROIs::chartClick);
    }
    bool outofbounds(QPointF pt) const noexcept
    {
        return !QRectF(0, 0, imgsize.width(), imgsize.height()).contains(pt);
    }
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
    int getIndex(const ROIShape *r) noexcept
    {
        const auto it = find(r);
        return it == rois.end() ? -1 : it - rois.begin();
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
    int getIndex(const TraceChartWidget *chart)
    {
        const auto it = find(chart);
        return it == rois.end() ? -1 : it - rois.begin();
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
    int getIndex(const TraceChartSeries *series)
    {
        const auto it = find(series);
        return it == rois.end() ? -1 : it - rois.begin();
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
    bool dispatchPressToSelector(QList<QGraphicsItem *> hititems, QPointF mappedpoint)
    {
        //  there's a very rare bug, where when drawing a poly it doesn't hit itself
        // this only happens on the initial draw, which means it's on the back of the stack
        if (!rois.empty() && rois.back()->graphicsShape->getShapeType() == ROIVert::SHAPE::POLYGON &&
            rois.back()->graphicsShape->getEditingVertex() >= 0)
        {
            return true;
        }

        for (auto &obj : hititems)
        {
            const bool isSelector = qgraphicsitem_cast<ROISelector *>(obj) != nullptr;
            auto par = obj->parentItem();
            ROIShape *roiGraphicsObject = par == nullptr ? nullptr : qgraphicsitem_cast<ROIShape *>(par);

            if (roiGraphicsObject != nullptr &&
                ((isSelector && roiGraphicsObject->isSelectVisible()) || // hit a selector
                 (roiGraphicsObject->getEditingVertex() >= 0)))          // closing a poly
            {
                roiGraphicsObject->doPress(mappedpoint.toPoint());
                return true;
            }
        }
        return false;
    }
    void selectPress(QList<QGraphicsItem *> hititems, const QMouseEvent *event)
    {
        const bool isShiftMod = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
        if (!hititems.isEmpty())
        {
            auto par = hititems[0]->parentItem();
            const ROIShape *roiGraphicsObject = qgraphicsitem_cast<ROIShape *>(par);
            if (roiGraphicsObject)
            {
                const size_t sel = static_cast<size_t>(getIndex(roiGraphicsObject));
                std::vector<size_t> inds = {sel};
                if (isShiftMod)
                {
                    inds = selectedROIs;
                    auto it = std::find(inds.begin(), inds.end(), sel);
                    if (it == inds.end())
                        inds.push_back(sel);
                    else
                        inds.erase(it);
                }
                setSelectedROIs(inds);
            }
            else if (!isShiftMod)
            {
                setSelectedROIs(std::vector<size_t>());
            }
        }
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
    impl->scene = iView->scene();
    impl->imgsize = iView->getImageSize();
    impl->traceview = tView;
    impl->videodata = vData;
    impl->par = this; // used for connects on push

    // Connect ridge chart with self
    connect(&tView->getRidgeChart(), &TraceChartWidget::chartClicked, this, &ROIs::chartClick);

    // connect with image view
    connect(iView, &ImageView::mousePressed, this, &ROIs::mousePress);
    connect(iView, &ImageView::keyPressed, this, &ROIs::keyPress);
    connect(iView, &ImageView::imageSizeUpdated, this, &ROIs::imageSizeUpdate);

    // connect with trace view
    connect(tView, &TraceViewWidget::chartClicked, this, &ROIs::chartClick);
    connect(tView, &TraceViewWidget::keyPressed, this, &ROIs::keyPress);
}
ROIs::~ROIs() = default;

void ROIs::mousePress(QList<QGraphicsItem *> hititems, const QPointF &mappedpoint, QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || impl->outofbounds(mappedpoint) || impl->dispatchPressToSelector(hititems, mappedpoint))
    {
        return;
    }

    if (impl->mousemode == ROIVert::SHAPE::SELECT)
    {
        impl->selectPress(hititems, event);
    }
    else
    {
        impl->pushROI(QPoint(std::floor(mappedpoint.x()), std::floor(mappedpoint.y())));
        impl->setSelectedROIs({impl->rois.size() - 1});
    }
}
void ROIs::keyPress(int key, Qt::KeyboardModifiers mods)
{
    if (key == Qt::Key::Key_Delete)
    {
        // delete all selected rois
        if (!impl->selectedROIs.empty())
        {
            std::vector<size_t> roistodelete = impl->selectedROIs;
            impl->deleteROIs(roistodelete);
        }
    }
    else if (key == Qt::Key::Key_A && mods == Qt::KeyboardModifier::ControlModifier)
    {
        // select all
        std::vector<size_t> inds(impl->rois.size());
        std::iota(inds.begin(), inds.end(), 0);
        impl->setSelectedROIs(inds);
    }
}
void ROIs::imageSizeUpdate(QSize newsize)
{
    impl->imgsize = newsize;
    for (auto &r : impl->rois)
    {
        r->graphicsShape->setBoundingRect(QRectF(0., 0., newsize.width(), newsize.height()));
    }
}

void ROIs::setROIShape(ROIVert::SHAPE shp) noexcept
{
    impl->mousemode = shp;
}

void ROIs::roiEdit(ROIVert::SHAPE, QRect, std::vector<QPoint>)
{
    // need to force an update after an edit completion to clean up artifact pixels outside of image rect.
    impl->scene->update();
    impl->updateYLimits();
}

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

void ROIs::deleteAllROIs()
{
    std::vector<size_t> inds(impl->rois.size());
    std::iota(inds.begin(), inds.end(), 0);
    impl->deleteROIs(inds);
}

void ROIs::chartClick(TraceChartWidget *chart, std::vector<TraceChartSeries *> series, Qt::KeyboardModifiers mods)
{

    int ind{-1};

    if (chart == &impl->traceview->getRidgeChart() && !series.empty())
    {
        ind = impl->getIndex(series.back());
    }
    else
    {
        ind = impl->getIndex(chart);
    }

    if (ind == -1 && mods == Qt::ShiftModifier)
    {
        return;
    }

    std::vector<size_t> inds;

    if (ind > -1)
    {
        if (mods == Qt::ShiftModifier)
        {
            inds = impl->selectedROIs;
            auto it = std::find(inds.begin(), inds.end(), ind);
            if (it == inds.end())
                inds.push_back(ind);
            else
                inds.erase(it);
        }
        else
        {
            inds.push_back(ind);
        }
    }
    impl->setSelectedROIs(inds);
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
        impl->pushROI(QPoint());
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
