#include <QMouseEvent>

#include "ROI/ROIController.h"
#include "ROI/ROIs.h"
#include "ROI/ROI.h"
#include "ROI/ROISelector.h"
#include "dockwidgets/TraceViewWidget.h"
#include "widgets/TraceChartWidget.h"
#include "ImageView.h"


// todo: consider moving selected in here

namespace {
    
    bool outofbounds(const QPointF pt, const QSize& imgsize) noexcept
    {
        return !QRectF(0, 0, imgsize.width(), imgsize.height()).contains(pt);
    }

    bool dispatchPressToSelector(QList<QGraphicsItem *> hititems, QPointF mappedpoint, const ROIs* rois)
    {
        //  there's a very rare bug, where when drawing a poly it doesn't hit itself
        // this only happens on the initial draw, which means it's on the back of the stack
        auto n = rois->getNROIs();
        ROI* backroi = n > 0 ? rois->getROI(n-1) : nullptr;
        
        if (backroi && 
            backroi->graphicsShape->getShapeType() == ROIVert::SHAPE::POLYGON &&
            backroi->graphicsShape->getEditingVertex() >= 0)
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

    void selectPress(QList<QGraphicsItem *> hititems, const QMouseEvent *event, ROIs* rois)
    {
        const bool isShiftMod = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
        if (!hititems.isEmpty())
        {
            auto par = hititems[0]->parentItem();
            const ROIShape *roiGraphicsObject = qgraphicsitem_cast<ROIShape *>(par);
            if (roiGraphicsObject)
            {
                const size_t sel = static_cast<size_t>(rois->getIndex(roiGraphicsObject));
                std::vector<size_t> inds = {sel};
                if (isShiftMod)
                {
                    inds = rois->getSelected();
                    auto it = std::find(inds.begin(), inds.end(), sel);
                    if (it == inds.end())
                        inds.push_back(sel);
                    else
                        inds.erase(it);
                }
                rois->setSelected(inds);
            }
            else if (!isShiftMod)
            {
                rois->setSelected(std::vector<size_t>());
            }
        }
    }
}

ROIController::ROIController(ROIs* r, TraceViewWidget* t, ImageView* i) : rois(r), tview(t), iview(i) {}

ROIController::~ROIController() {}

void ROIController::keyPress(int key, Qt::KeyboardModifiers mods)
{
    if (key == Qt::Key::Key_Delete)
    {
        // delete all selected rois
        auto selrois = rois->getSelected();
        if (!selrois.empty())
        {
            rois->deleteROIs(selrois);
        }
    }
    else if (key == Qt::Key::Key_A && mods == Qt::KeyboardModifier::ControlModifier)
    {
        // select all
        std::vector<size_t> inds(rois->getNROIs());
        std::iota(inds.begin(), inds.end(), 0);
        rois->setSelected(inds);
    }
}

void ROIController::chartClick(TraceChartWidget *chart, std::vector<TraceChartSeries *> series, Qt::KeyboardModifiers mods)
{
    int ind{-1};

    if (chart == &tview->getRidgeChart() && !series.empty())
    {
        ind = rois->getIndex(series.back());
    }
    else
    {
        ind = rois->getIndex(chart);
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
            inds = rois->getSelected();
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
    rois->setSelected(inds);
}

void ROIController::mousePress(QList<QGraphicsItem *> hititems, const QPointF &mappedpoint, QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || outofbounds(mappedpoint, iview->getImageSize()) || dispatchPressToSelector(hititems, mappedpoint, rois))
    {
        return;
    }

    if (mousemode == ROIVert::SHAPE::SELECT)
    {
        selectPress(hititems, event, rois);
    }
    else
    {
        rois->pushROI(QPoint(std::floor(mappedpoint.x()), std::floor(mappedpoint.y())), mousemode);
        rois->setSelected({rois->getNROIs() - 1});
    }
}

void ROIController::setROIShape(ROIVert::SHAPE shp) noexcept
{
    mousemode = shp;
}
void ROIController::imageSizeUpdate(QSize newsize)
{
    for (size_t i = 0; i < rois->getNROIs(); ++i) {
        rois->getROI(i)->graphicsShape->setBoundingRect(QRectF(0., 0., newsize.width(), newsize.height()));
    }
}
void ROIController::roiEdit(ROIVert::SHAPE, QRect, std::vector<QPoint>)
{
    // need to force an update after an edit completion to clean up artifact pixels outside of image rect.
    rois->update();
}
