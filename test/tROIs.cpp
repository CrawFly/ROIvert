#include <QtTest/QtTest>

#include "tROIs.h"
#include "ROI/ROIs.h"
#include "ROI/ROISelector.h"
#include "dockwidgets/TraceViewWidget.h"


void tROIs::initTestCase() {
    data = new VideoData;
    QStringList f = { "roiverttestdata.tiff" };
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


namespace {
    /*
    void addroi(ROIs* r, ROIVert::SHAPE shp, std::vector<QPointF> coords) {
        r->setROIShape(shp);
        QMouseEvent evt(QEvent::Type::MouseButtonPress, QPointF(), Qt::MouseButton::LeftButton, Qt::MouseButtons(), Qt::KeyboardModifier::NoModifier);

        if (shp != ROIVert::SHAPE::POLYGON) {
            for (const auto& pt : coords) {
                r->mousePress(QList<QGraphicsItem*>(), pt, &evt);
            }
        }
        else {

        }
    }
    */
}

void tROIs::taddroi() {
    // Add an roi of each type
    /*
    addroi(rois, ROIVert::SHAPE::RECTANGLE, { {0,0}, {2,2} });
    QCOMPARE(rois->getNROIs(), 1);
    
    addroi(rois, ROIVert::SHAPE::ELLIPSE, { {0,0}, {3,3} });
    QCOMPARE(rois->getNROIs(), 2);
    
    addroi(rois, ROIVert::SHAPE::POLYGON, { {0,0} });
    auto items = iview->items();
    for (auto& item : items) {
        auto a = qgraphicsitem_cast<ROISelector*>(item);
        qDebug() << "null" << (a == nullptr);
        if (a) {
            qDebug() << "vis" << a->isVisibleTo(item->parentItem());
        }
    }
    */

    //addroi(rois, ROIVert::SHAPE::POLYGON, { {0,0}, {0,2}, {2,0}, {0,0} });
    //QCOMPARE(rois->getNROIs(), 3);
}
