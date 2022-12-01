#include <QMouseEvent>
#include <QDebug>

#include "ROI/ROIController.h"
#include "ROI/ROIs.h"
#include "ROI/ROI.h"
#include "ROI/ROISelector.h"
#include "dockwidgets/TraceViewWidget.h"
#include "widgets/TraceChartWidget.h"
#include "ImageView.h"

namespace {
    bool outofbounds(const QPointF pt, const QSize& imgsize) noexcept
    {
        return !QRectF(0, 0, imgsize.width(), imgsize.height()).contains(pt);
    }

    bool dispatchPressToSelector(QList<QGraphicsItem*> hititems, QPointF mappedpoint, const ROIs* rois)
    {
        //  there's a very rare bug, where when drawing a poly it doesn't hit itself
        // this only happens on the initial draw, which means it's on the back of the stack
        auto n = rois->size();
        const ROI* backroi = n > 0 ? &((*rois)[n - 1]) : nullptr;

        if (backroi &&
            backroi->graphicsShape->getShapeType() == ROIVert::SHAPE::POLYGON &&
            backroi->graphicsShape->getEditingVertex() >= 0)
        {
            return true;
        }

        for (auto& obj : hititems)
        {
            const bool isSelector = qgraphicsitem_cast<ROISelector*>(obj) != nullptr;
            auto par = obj->parentItem();
            ROIShape* roiGraphicsObject = par == nullptr ? nullptr : qgraphicsitem_cast<ROIShape*>(par);

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
}

struct ROIController::pimpl
{
    pimpl(ROIs* r, TraceViewWidget* t, ImageView* i) : rois(r), tview(t), iview(i) { }

    ROIs* rois;
    TraceViewWidget* tview;
    ImageView* iview;
    ROIVert::SHAPE mousemode{ ROIVert::SHAPE::ELLIPSE };

    void selectPress(QList<QGraphicsItem*> hititems, const QMouseEvent* event)
    {
        // this is mouse presses when selecting
        const bool isShiftMod = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
        if (!hititems.isEmpty())
        {
            auto par = hititems[0]->parentItem();
            const ROIShape* roiGraphicsObject = qgraphicsitem_cast<ROIShape*>(par);
            if (roiGraphicsObject)
            {
                const size_t sel = static_cast<size_t>(rois->getIndex(roiGraphicsObject));
                std::vector<size_t> inds = { sel };
                if (isShiftMod)
                {
                    inds = getSelected();
                    auto it = std::find(inds.begin(), inds.end(), sel);
                    if (it == inds.end())
                        inds.push_back(sel);
                    else
                        inds.erase(it);
                }
                setSelected(inds);
            }
            else if (!isShiftMod)
            {
                setSelected(std::vector<size_t>());
            }
        }
    }

    void keyPress(int key, Qt::KeyboardModifiers mods) {
        if (key == Qt::Key::Key_Delete)
        {
            // delete all selected rois
            rois->deleteROIs(getSelected());
        }
        else if (key == Qt::Key::Key_A && mods == Qt::KeyboardModifier::ControlModifier)
        {
            // select all
            std::vector<size_t> inds(rois->size());
            std::iota(inds.begin(), inds.end(), 0);
            setSelected(inds);
        }
    }
    void chartClick(TraceChartWidget* chart, std::vector<TraceChartSeries*> series, Qt::KeyboardModifiers mods) {
        int ind{ -1 };

        if (chart == &tview->getRidgeChart() && !series.empty()) ind = rois->getIndex(series.back());
        else ind = rois->getIndex(chart);

        if (ind == -1 && mods == Qt::ShiftModifier) return;

        std::vector<size_t> inds;
        if (ind > -1)
        {
            if (mods == Qt::ShiftModifier)
            {
                inds = getSelected();
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
        setSelected(inds);
    }
    void mousePress(QList<QGraphicsItem*> hititems, const QPointF& mappedpoint, QMouseEvent* event) {
        if (event->button() != Qt::LeftButton || outofbounds(mappedpoint, iview->getImageSize()) || dispatchPressToSelector(hititems, mappedpoint, rois))
        {
            return;
        }

        if (mousemode == ROIVert::SHAPE::SELECT)
        {
            selectPress(hititems, event);
        }
        else
        {
            rois->pushROI(QPoint(std::floor(mappedpoint.x()), std::floor(mappedpoint.y())), mousemode);
            setSelected({ rois->size() - 1 });
        }
    }

    void setSelected(std::vector<size_t> inds) {
        // could do set difference, but it's easy and fast enough to just unselect and reselect
        for (size_t i = 0; i < rois->size(); ++i) {
            (*rois)[i].setSelected(false);
        }
        for (auto& i : inds) {
            if (i < rois->size()) {
                (*rois)[i].setSelected(true);
            }
        }
        tview->update();

        if (!inds.empty() && inds.back() < rois->size())
        {
            tview->scrollToChart((*rois)[inds.back()].Trace->getTraceChart());
        }
        emit rois->selectionChanged(inds);
    }
    std::vector<size_t> getSelected() const noexcept {
        auto ret = std::vector<size_t>();
        for (size_t i = 0; i < rois->size(); ++i) {
            if ((*rois)[i].getSelected()) {
                ret.push_back(i);
            }
        }
        return ret;
    }
};

ROIController::ROIController(ROIs* r, TraceViewWidget* t, ImageView* i) : impl(std::make_unique<pimpl>(r, t, i)) { }
ROIController::~ROIController() { }

void ROIController::keyPress(int key, Qt::KeyboardModifiers mods)
{
    impl->keyPress(key, mods);
}
void ROIController::chartClick(TraceChartWidget* chart, std::vector<TraceChartSeries*> series, Qt::KeyboardModifiers mods)
{
    impl->chartClick(chart, series, mods);
}
void ROIController::mousePress(QList<QGraphicsItem*> hititems, const QPointF& mappedpoint, QMouseEvent* event)
{
    impl->mousePress(hititems, mappedpoint, event);
}

void ROIController::setROIShape(ROIVert::SHAPE shp) noexcept
{
    impl->mousemode = shp;
}
void ROIController::imageSizeUpdate(QSize newsize)
{
    for (size_t i = 0; i < impl->rois->size(); ++i) {
        (*impl->rois)[i].graphicsShape->setBoundingRect(QRectF(0., 0., newsize.width(), newsize.height()));
    }
}
void ROIController::roiEdit(ROIVert::SHAPE, QRect, std::vector<QPoint>)
{
    impl->rois->update();
}

void ROIController::setSelected(std::vector<size_t> inds) {
    impl->setSelected(inds);
}
std::vector<size_t> ROIController::getSelected() const noexcept {
    return impl->getSelected();
}