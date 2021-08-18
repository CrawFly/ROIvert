#include "ROI\ROIs.h"
#include <QDebug>
#include <QGraphicsItem>
#include <QMouseEvent>

#include "ROI\ROI.h"
#include "ROI\ROISelector.h"
#include "ROI\ROIStyle.h"
#include "ChartStyle.h"
#include "widgets/TraceChartWidget.h"
#include "widgets/RidgeLineWidget.h"

struct ROIs::pimpl {
    QGraphicsScene* scene{ nullptr };
    TraceView* traceview{ nullptr };
    VideoData* videodata{ nullptr };

    std::vector<std::unique_ptr<ROI>> rois; // todo: consider whether ROI could be a unique_ptr?
    ROIPalette pal;
    ROIStyle coreStyle;
    
    QSize imgsize;
    std::vector<size_t> selectedROIs; 

    ROIVert::SHAPE mousemode{ ROIVert::SHAPE::ELLIPSE };

    void pushROI(QPoint pos) {
        //todo: more careful work for style needed here...
        ROIStyle rs = coreStyle;
        ChartStyle cs = traceview->getCoreChartStyle();
        rs.setColor(pal.getPaletteColor(rois.size()));
        rois.push_back(std::make_unique<ROI>(scene, traceview, videodata, mousemode, imgsize, rs, cs));
        
        auto& gObj = rois.back()->graphicsShape;
        auto& tObj = rois.back()->Trace;
        

        gObj->setVertices({ pos, pos });
        gObj->setEditingVertex(1);
        gObj->grabMouse();

        connect(gObj.get(), &ROIShape::roiEdited, tObj.get(), &ROITrace::updateTrace);
        connect(tObj->getTraceChart(), &TraceChartWidget::chartClicked, par, &ROIs::chartClick);
    }

    bool outofbounds(QPointF pt) const noexcept{
        return !QRectF(0, 0, imgsize.width(), imgsize.height()).contains(pt);
    }

    void setSelectedROIs(std::vector<size_t> inds) {
        for (auto& ind : selectedROIs) {
            if (ind < rois.size()) {
                rois.at(ind)->graphicsShape->setSelectVisible(false);
                rois.at(ind)->roistyle->setSelected(false);
                rois.at(ind)->Trace->getLineSeries()->setHighlighted(false);
                rois.at(ind)->Trace->getRidgeSeries()->setHighlighted(false);
                rois.at(ind)->Trace->getTraceChart()->update();
            }
        }
        selectedROIs = inds;
        for (auto& ind : selectedROIs) {
            if (ind < rois.size()) {
                rois.at(ind)->graphicsShape->setSelectVisible(true);
                rois.at(ind)->roistyle->setSelected(true);
                rois.at(ind)->Trace->getLineSeries()->setHighlighted(true);
                rois.at(ind)->Trace->getRidgeSeries()->setHighlighted(true);
                rois.at(ind)->Trace->getTraceChart()->update();
            }
        }
        traceview->update();
        if (!selectedROIs.empty() && selectedROIs.back()<rois.size()) {
            traceview->scrollToChart(rois.at(selectedROIs.back())->Trace->getTraceChart());
        }
    }
    
    std::vector<std::unique_ptr<ROI>>::iterator find(const ROIShape* r) noexcept{
        for(auto it = rois.begin(); it<rois.end();++it){
            if ((*it)->graphicsShape.get() == r) {
                return it;
            }
        }
        return rois.end();
    }
    int getIndex(const ROIShape* r) noexcept {
        const auto it = find(r);
        return it == rois.end() ? -1 : it - rois.begin();
    }

    std::vector<std::unique_ptr<ROI>>::iterator find(const TraceChartWidget* chart) noexcept{
        for(auto it = rois.begin(); it<rois.end();++it){
            if ((*it)->Trace->getTraceChart() == chart) {
                return it;
            }
        }
        return rois.end();
    }
    int getIndex(const TraceChartWidget* chart) {
        const auto it = find(chart);
        return it == rois.end() ? -1 : it - rois.begin();
        
    }
    
    std::vector<std::unique_ptr<ROI>>::iterator find(const TraceChartSeries* series) noexcept{
        for(auto it = rois.begin(); it<rois.end();++it){
            if ((*it)->Trace->getRidgeSeries() == series) {
                return it;
            }
        }
        return rois.end();
    }
    int getIndex(const TraceChartSeries* series) {
        const auto it = find(series);
        return it == rois.end() ? -1 : it - rois.begin();

    }


    void deleteROIs(std::vector<size_t> inds) {
        // Unselect:
        std::vector<size_t> selected = selectedROIs;
        std::sort(selected.begin(), selected.end());
        std::sort(inds.begin(), inds.end());
        std::vector<size_t> updatedSelected;
        std::set_difference(selected.begin(), selected.end(), inds.begin(), inds.end(),
            std::inserter(updatedSelected, updatedSelected.begin()));
        setSelectedROIs(updatedSelected);

        for (auto& ind : inds) {
            rois[ind] = nullptr;
        }

        // clean up the vector
        for (auto it = rois.begin(); it != rois.end(); ) {
            if (*it == nullptr) {
                it = rois.erase(it);
            }
            else {
                ++it;
            }
        }
        scene->update();
    }
    
    bool dispatchPressToSelector(QList<QGraphicsItem*> hititems, QPointF mappedpoint) {
        //  there's a very rare bug, where when drawing a poly it doesn't hit itself
        // this only happens on the initial draw, which means it's on the back of the stack
        if (!rois.empty() && rois.back()->graphicsShape->getShapeType() == ROIVert::SHAPE::POLYGON &&
            rois.back()->graphicsShape->getEditingVertex() >= 0) {
            return true;
        }

        for (auto &obj : hititems) {
            const bool isSelector = qgraphicsitem_cast<ROISelector*>(obj) != nullptr;
            auto par = obj->parentItem();
            ROIShape* roiGraphicsObject = par==nullptr ? nullptr : qgraphicsitem_cast<ROIShape*>(par);
            
            if (roiGraphicsObject != nullptr &&
                ((isSelector && roiGraphicsObject->isSelectVisible()) ||    // hit a selector
                (roiGraphicsObject->getEditingVertex() >= 0)))              // closing a poly
                {
                    roiGraphicsObject->doPress(mappedpoint.toPoint());
                    return true;
                }
        }
        return false;
    }

    void selectPress(QList<QGraphicsItem*> hititems, const QMouseEvent* event) {
        const bool isShiftMod = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
        if (!hititems.isEmpty()) {
            auto par = hititems[0]->parentItem();
            const ROIShape* roiGraphicsObject = qgraphicsitem_cast<ROIShape*>(par);
            if (roiGraphicsObject) {
                const size_t sel = static_cast<size_t>(getIndex(roiGraphicsObject));
                std::vector<size_t> inds = { sel };
                if (isShiftMod) {
                    inds = selectedROIs;
                    auto it = std::find(inds.begin(), inds.end(), sel);
                    if (it == inds.end()) inds.push_back(sel);
                    else inds.erase(it);
                }
                setSelectedROIs(inds);
            }
            else if (!isShiftMod) {
                setSelectedROIs(std::vector<size_t>());
            }
        }
    }

    ROIs* par{ nullptr };
};


ROIs::ROIs(ImageView* iView, TraceView* tView, VideoData* vData) {
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
    connect(tView, &TraceView::chartClicked, this, &ROIs::chartClick);
    connect(tView, &TraceView::keyPressed, this, &ROIs::keyPress);

}
ROIs::~ROIs() = default;

void ROIs::mousePress(QList<QGraphicsItem*> hititems, const QPointF& mappedpoint, QMouseEvent* event) {
    // todo: would be nice to make this live in mousePressFromImage or something like that so that we can 
    //      also have a version for charts...
    if (event->button() != Qt::LeftButton || impl->outofbounds(mappedpoint)) {
        return;
    }

    // Check for a passthrough to an ROI object, in which case return
    if (impl->dispatchPressToSelector(hititems, mappedpoint)) {
        return;
    }

    
    if (impl->mousemode == ROIVert::SHAPE::SELECT) {
        impl->selectPress(hititems, event);
    }
    else {
        impl->pushROI(QPoint(std::floor(mappedpoint.x()), std::floor(mappedpoint.y())));
        auto& it = impl->rois.back();        
        connect(it->graphicsShape.get(), &ROIShape::roiEdited, this, &ROIs::roiEdit);
        // TODO: examine this connection, how it should work with new ptr mechanism is unclear...
        //connect(it->roistyle.get(), &ROIStyle::StyleChanged, it->graphicsShape.get(), &ROIShape::updateStyle);
        impl->setSelectedROIs({ impl->rois.size() - 1 });
    }
    
}
void ROIs::keyPress(int key, Qt::KeyboardModifiers mods){
    if (key == Qt::Key::Key_Delete) {
        // delete all selected rois
        if (!impl->selectedROIs.empty()) {
            std::vector<size_t> roistodelete = impl->selectedROIs;
            impl->deleteROIs(roistodelete);
        }
    }
    else if (key == Qt::Key::Key_A && mods == Qt::KeyboardModifier::ControlModifier) {
        // select all
        std::vector<size_t> inds(impl->rois.size());
        std::iota(inds.begin(), inds.end(), 0);
        impl->setSelectedROIs(inds);
    }
}
void ROIs::imageSizeUpdate(QSize newsize){
    impl->imgsize = newsize;
    for (auto& r : impl->rois) {
        r->graphicsShape->setBoundingRect(QRectF(0., 0., newsize.width(), newsize.height()));
    }
}

void ROIs::setROIShape(ROIVert::SHAPE shp) noexcept {
    impl->mousemode = shp;
}

void ROIs::roiEdit(ROIVert::SHAPE, QRect, std::vector<QPoint>) {
    // need to force an update after an edit completion to clean up artifact pixels outside of image rect.
    impl->scene->update();
}

std::vector<size_t> ROIs::getSelected() const noexcept {
    return impl->selectedROIs;
};

ROIStyle* ROIs::getROIStyle(size_t ind) const noexcept {
    // todo: eval whether this is needed...
    if (ind > impl->rois.size()) {
        return nullptr;
    }
    return impl->rois[ind]->roistyle.get();
}

void ROIs::setColorBySelect(bool yesno) {
    // todo: eval whether this is needed...or if a more general approach is warranted./..
    impl->coreStyle.setColorBySelected(yesno);
    for (auto &r : impl->rois) {
        r->roistyle->setColorBySelected(yesno);
    }
}

void ROIs::updateROITraces() {
    for (auto& r : impl->rois) {
        r->Trace->update();
    }
}

size_t ROIs::getNROIs() const noexcept { return impl->rois.size(); }

void ROIs::deleteAllROIs() {
    std::vector<size_t> inds(impl->rois.size());
    std::iota(inds.begin(), inds.end(), 0);
    impl->deleteROIs(inds);
}

void ROIs::chartClick(TraceChartWidget* chart, std::vector<TraceChartSeries*> series, Qt::KeyboardModifiers mods) {

    int ind{ -1 };

    if (chart == &impl->traceview->getRidgeChart() && !series.empty()) {
        ind = impl->getIndex(series.back());
    }
    else {
        ind = impl->getIndex(chart);
    }
    
    if (ind == -1 && mods == Qt::ShiftModifier) {
        return;
    }

    std::vector<size_t> inds;
    
    if (ind > -1) {
        if (mods == Qt::ShiftModifier) {
            inds = impl->selectedROIs;
            auto it = std::find(inds.begin(), inds.end(), ind);
            if (it == inds.end()) inds.push_back(ind);
            else inds.erase(it);
        }
        else {
            inds.push_back(ind);
        }
    }
    impl->setSelectedROIs(inds);
}

std::vector<std::vector<float>> ROIs::getTraces(std::vector<size_t> inds) const {
    auto out = std::vector<std::vector<float>>();
    out.reserve(inds.size());

    for (auto &ind : inds) {
        if (ind < impl->rois.size()) {
            out.push_back(impl->rois.at(ind)->Trace->getTrace());
        }
    }
    return out;
}