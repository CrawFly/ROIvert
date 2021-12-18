#include <QtTest/QtTest>

#include "tROIs.h"
#include "ROI/ROIs.h"
#include "ROI/ROISelector.h"
#include "dockwidgets/TraceViewWidget.h"
#include "VideoData.h"
#include "ImageView.h"
#include "widgets/TraceChartWidget.h"

void tROIs::initTestCase() {
    data = new VideoData;
    QStringList f = { QDir::currentPath() + "/test_resources/roiverttestdata.tiff" };
    data->load(f, 1, 1, false);
    
    tview = new TraceViewWidget;
    iview = new ImageView;
}

void tROIs::init() {
    rois = new ROIs(iview, tview, data);
}
void tROIs::cleanup() {
    delete rois;
    rois = nullptr;
}
void tROIs::cleanupTestCase() {
    delete tview;
    delete iview;
    delete data;
}

void tROIs::taddroi() {
    
    {
        rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
        QCOMPARE(rois->size(), 1);    
        auto& roi = (*rois)[0];
        auto& roishape = roi.graphicsShape;
        QCOMPARE(roishape->getEditingVertex(), 1);
        QCOMPARE(roishape->getVertices()[0], QPoint(0, 0));
        QCOMPARE(roishape->getVertices()[1], QPoint(0, 0));
        QCOMPARE(roishape->getShapeType(), ROIVert::SHAPE::RECTANGLE);
        roishape->setVertices({ QPoint(0, 0), QPoint(2, 2) });
        emit roishape->roiEdited(ROIVert::SHAPE::RECTANGLE, roishape->getTightBoundingBox(), roishape->getVertices());
        auto trace = roi.Trace->getTrace();    
        QCOMPARE(trace.size(), data->getNFrames());

        auto mattrace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, roishape->getTightBoundingBox(), roishape->getVertices());
        for (size_t i = 0; i < trace.size(); ++i) {
            QCOMPARE(mattrace.at<float>(i), trace[i]);
        }
    }
    {
        rois->pushROI({ 0, 0 }, ROIVert::SHAPE::ELLIPSE);
        QCOMPARE(rois->size(), 2);
        auto& roi = (*rois)[1];
        auto& roishape = roi.graphicsShape;
        QCOMPARE(roishape->getVertices()[0], QPoint(0, 0));
        QCOMPARE(roishape->getVertices()[1], QPoint(0, 0));
        QCOMPARE(roishape->getShapeType(), ROIVert::SHAPE::ELLIPSE);
        QCOMPARE(roishape->getEditingVertex(), 1);
    }
    {
        rois->pushROI({ 0, 0 }, ROIVert::SHAPE::POLYGON);
        QCOMPARE(rois->size(), 3);
        auto& roi = (*rois)[2];
        auto& roishape = roi.graphicsShape;
        QCOMPARE(roishape->getVertices()[0], QPoint(0, 0));
        QCOMPARE(roishape->getVertices()[1], QPoint(0, 0));
        QCOMPARE(roishape->getShapeType(), ROIVert::SHAPE::POLYGON);
        QCOMPARE(roishape->getEditingVertex(), 1);
        rois->pushROI({ 0, 0 }, ROIVert::SHAPE::POLYGON);
    }
}

void tROIs::tdeleteroi() {
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    QCOMPARE(rois->size(), 4);
    auto roi0add = &(*rois)[0];
    auto roi1add = &(*rois)[1];
    auto roi2add = &(*rois)[2];
    auto roi3add = &(*rois)[3];

    rois->setSelected({ 0, 3 });
    rois->deleteROIs({0, 2});
    
    QCOMPARE(rois->getSelected(), { 1 });
    QCOMPARE(rois->size(), 2);

    // the remaining rois should be the old 1 and 3
    QCOMPARE(&(*rois)[0], roi1add);
    QCOMPARE(&(*rois)[1], roi3add);

    rois->deleteAllROIs();
    QCOMPARE(rois->getSelected(), { });
    QCOMPARE(rois->size(), 0);
    
}
void tROIs::tindexfinders() {
    
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);

    for (size_t i = 0; i < rois->size(); ++i) {
        QCOMPARE(rois->getIndex((*rois)[i].graphicsShape.get()), i);
        QCOMPARE(rois->getIndex((*rois)[i].Trace->getTraceChart()), i);
        QCOMPARE(rois->getIndex((*rois)[i].Trace->getRidgeSeries()), i);
    }
     
    QCOMPARE(rois->getIndex(std::unique_ptr<ROIShape>().get()), -1);
    QCOMPARE(rois->getIndex(std::unique_ptr<TraceChartWidget>().get()), -1);
    QCOMPARE(rois->getIndex(std::unique_ptr<TraceChartSeries>().get()), -1);
}
void tROIs::tselected() {
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);

    rois->setSelected({ 0, 2 });
    QCOMPARE(rois->getSelected(), std::vector<size_t>({ 0, 2 }));
    QVERIFY((*rois)[0].graphicsShape->isSelectVisible());
    QVERIFY(!(*rois)[1].graphicsShape->isSelectVisible());
    QVERIFY((*rois)[2].graphicsShape->isSelectVisible());
    QVERIFY(!(*rois)[3].graphicsShape->isSelectVisible());

    
    rois->setSelected({ 1, 3 });
    QCOMPARE(rois->getSelected(), std::vector<size_t>({ 1, 3 }));
    QVERIFY(!(*rois)[0].graphicsShape->isSelectVisible());
    QVERIFY((*rois)[1].graphicsShape->isSelectVisible());
    QVERIFY(!(*rois)[2].graphicsShape->isSelectVisible());
    QVERIFY((*rois)[3].graphicsShape->isSelectVisible());
}

void tROIs::tmatchy() {
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);

    rois->setMatchYAxes(true);
    QVERIFY(rois->getMatchYAxes());
    QCOMPARE((*rois)[0].linechartstyle->getLimitStyle(), ROIVert::LIMITSTYLE::MANAGED);
    QCOMPARE((*rois)[1].linechartstyle->getLimitStyle(), ROIVert::LIMITSTYLE::MANAGED);
    
    rois->setMatchYAxes(false);
    QVERIFY(!rois->getMatchYAxes());
    QCOMPARE((*rois)[0].linechartstyle->getLimitStyle(), ROIVert::LIMITSTYLE::AUTO);
    QCOMPARE((*rois)[1].linechartstyle->getLimitStyle(), ROIVert::LIMITSTYLE::AUTO);

}

void tROIs::tpalettepush() {
    //**note: at time of writing, palette isn't settable, testing with defaults
    ROIPalette pal;
    
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);

    for (size_t i = 0; i < 4; ++i) {
        QCOMPARE((*rois)[i].roistyle->getLineColor(), pal.getPaletteColor(i));
    }
}

void tROIs::tchartclicked() {
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);
    rois->pushROI({ 0, 0 }, ROIVert::SHAPE::RECTANGLE);

    {
        auto chart = (*rois)[1].Trace->getTraceChart();
        auto series = (*rois)[1].Trace->getLineSeries();
        emit chart->chartClicked(chart, { series }, Qt::KeyboardModifier::NoModifier);
        QCOMPARE(rois->getSelected(), { 1 });
    }

    {
        auto chart = &tview->getRidgeChart();
        auto series = (*rois)[0].Trace->getRidgeSeries();
        emit chart->chartClicked(chart, { series }, Qt::KeyboardModifier::NoModifier);
        QCOMPARE(rois->getSelected(), { 0 });
    }
}
