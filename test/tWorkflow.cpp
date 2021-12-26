#include "tWorkflow.h"
#include <QtTest/QtTest>
#include "opencv2/opencv.hpp"

#include <QGUIApplication>
#include <QActionGroup>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>

#include "roivert.h"
#include "dockwidgets/FileIOWidget.h"
#include "dockwidgets/StyleWidget.h"
#include "dockwidgets/TraceViewWidget.h"
#include "dockwidgets/ImageSettingsWidget.h"
#include "dockwidgets/ImageDataWidget.h"
#include "ImageView.h"
#include "VideoData.h"
#include "ROI/ROIShape.h"
#include "ROI/ROIs.h"
#include "widgets/TraceChartWidget.h"
#include "ZoomPan.h"

#include <QElapsedTimer>

namespace dw {
    ImageDataWidget* imagedata(Roivert* r) { return r->findChild<ImageDataWidget*>(); }
    ImageSettingsWidget* imagesettings(Roivert* r) { return r->findChild<ImageSettingsWidget*>(); };
    StyleWidget* style(Roivert* r) { return r->findChild<StyleWidget*>(); };
    FileIOWidget* fileio(Roivert* r) { return r->findChild<FileIOWidget*>(); };
    TraceViewWidget* tview(Roivert* r) { return r->findChild<TraceViewWidget*>(); };

    void validate(Roivert* r) {
        QVERIFY(dw::imagedata(r) != nullptr);
        QVERIFY(dw::imagesettings(r) != nullptr);
        QVERIFY(dw::style(r) != nullptr);
        QVERIFY(dw::fileio(r) != nullptr);
        QVERIFY(dw::tview(r) != nullptr);
    }
    void hide(Roivert* r) {
        imagedata(r)->setVisible(false);
        imagesettings(r)->setVisible(false);
        style(r)->setVisible(false);
        fileio(r)->setVisible(false);
        tview(r)->setVisible(false);
    }
};
namespace {
    VideoData* vdata(Roivert* r) { return r->findChild<VideoData*>(); };
    ImageView* iview(Roivert* r) { return r->findChild<ImageView*>(); };
    ROIs* rois(Roivert* r) { return r->findChild<ROIs*>(); };

    void pause(int dur) {
        QElapsedTimer t;
        t.start();
        while (t.elapsed() < dur) {
            qApp->processEvents();
        }
    }
    void update() {
        qApp->processEvents();
    }

    void loaddataset(Roivert* r, int downtime = 1, int downspace = 1, int fr = 7) {
        auto optFolder = dw::imagedata(r)->findChild<QRadioButton*>("optFolder");
        auto optFile = dw::imagedata(r)->findChild<QRadioButton*>("optFile");
        auto txtFilePath = dw::imagedata(r)->findChild<QLineEdit*>("txtFilePath");
        auto cmdLoad = dw::imagedata(r)->findChild<QPushButton*>("cmdLoad");
        auto spinDownTime = dw::imagedata(r)->findChild<QSpinBox*>("spinDownTime");
        auto spinDownSpace = dw::imagedata(r)->findChild<QSpinBox*>("spinDownSpace");
        auto spinFrameRate = dw::imagedata(r)->findChild<QDoubleSpinBox*>("spinFrameRate");

        QVERIFY(optFolder);
        QVERIFY(optFile);
        QVERIFY(txtFilePath);
        QVERIFY(cmdLoad);
        QVERIFY(spinDownTime);
        QVERIFY(spinDownSpace);
        QVERIFY(spinFrameRate);

        optFolder->setChecked(false);
        optFile->setChecked(true);
        spinDownTime->setValue(downtime);
        spinDownSpace->setValue(downspace);
        spinFrameRate->setValue(fr);
        txtFilePath->setText(TEST_RESOURCE_DIR "/roiverttestdata.tiff");
        update();
        cmdLoad->click();
        update();
    }

    void selctshapeaction(Roivert* r, ROIVert::SHAPE s) {
        auto actiongroup = r->findChild<QActionGroup*>();
        QVERIFY(actiongroup);
        QAction* act = nullptr;

        switch (s)
        {
        case ROIVert::SHAPE::RECTANGLE:
            act = actiongroup->findChild<QAction*>("actROIRect");
            break;
        case ROIVert::SHAPE::ELLIPSE:
            act = actiongroup->findChild<QAction*>("actROIEllipse");
            break;
        case ROIVert::SHAPE::POLYGON:
            act = actiongroup->findChild<QAction*>("actROIPoly");
            break;
        case ROIVert::SHAPE::SELECT:
            act = actiongroup->findChild<QAction*>("actROISelect");
            break;
        default:
            break;
        }
        QVERIFY(act);
        act->setChecked(true);
        act->triggered(act);
    }

    void makeroi_nonpoly(Roivert* r, ROIVert::SHAPE s, QPointF v1, QPointF v2) {
        QTest::mouseMove(iview(r));

        selctshapeaction(r, s);
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(v1.x(), v1.y()));
        QTest::mouseMove(iview(r), iview(r)->mapFromScene(v2.x(), v2.y()));
        pause(50);
        QTest::mouseRelease(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(v2.x(), v2.y()));
        pause(50);
    }

    void makeroi_poly(Roivert* r, std::vector<QPointF> verts) {
        QTest::mouseMove(iview(r));
        selctshapeaction(r, ROIVert::SHAPE::POLYGON);
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(verts[0].x(), verts[0].y()));

        for (size_t i = 1; i < verts.size(); ++i) {
            QTest::mouseMove(iview(r), iview(r)->mapFromScene(verts[i].x(), verts[i].y()));
            QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(verts[i].x(), verts[i].y()));
            pause(50);
            QTest::mouseRelease(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(verts[i].x(), verts[i].y()));
        }

        // complete the ROI:
        QTest::mouseMove(iview(r), iview(r)->mapFromScene(verts[0].x(), verts[0].y()));
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(verts[0].x(), verts[0].y()));
        pause(50);
        QTest::mouseRelease(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(verts[0].x(), verts[0].y()));


    }
}

void tWorkflow::init() {
    r = new Roivert;
    r->show();
    r->setInitialSettings(false);
    dw::hide(r);
    // note: the size of the window seems to influence a rounding error (test-only) that varies across display resolution
    r->resize(1000, 1000); 
    r->activateWindow();
    r->move(1, 1);
    update();

    dw::validate(r);
}
void tWorkflow::cleanup() {
    delete(r);
}

void tWorkflow::tload() {
    loaddataset(r);
    QTRY_COMPARE_WITH_TIMEOUT(vdata(r)->getNFrames(), 7, 1000);
    QCOMPARE(vdata(r)->getdsSpace(), 1);
    QCOMPARE(vdata(r)->getdsTime(), 1);
    QCOMPARE(vdata(r)->get(false, 0, 0).at<uint8_t>(0), 208);
    QCOMPARE(vdata(r)->get(true, 0, 0).at<uint8_t>(0), 161);
    QCOMPARE(vdata(r)->getTMax(), 1);
    QCOMPARE(vdata(r)->getWidth(), 6);
    QCOMPARE(vdata(r)->getHeight(), 5);

    // changing framerate should have an instant effect
    auto spinFrameRate = dw::imagedata(r)->findChild<QDoubleSpinBox*>("spinFrameRate");
    spinFrameRate->setValue(14);
    QCOMPARE(vdata(r)->getTMax(), .5);

    loaddataset(r, 2);
    QTRY_COMPARE_WITH_TIMEOUT(vdata(r)->getNFrames(), 3, 1000);
    QCOMPARE(vdata(r)->getdsTime(), 2);
    QCOMPARE(vdata(r)->getdsSpace(), 1);
    QCOMPARE(vdata(r)->getWidth(), 6);
    QCOMPARE(vdata(r)->getHeight(), 5);

    loaddataset(r, 1, 2);
    QTRY_COMPARE_WITH_TIMEOUT(vdata(r)->getNFrames(), 7, 1000);
    QCOMPARE(vdata(r)->getdsTime(), 1);
    QCOMPARE(vdata(r)->getdsSpace(), 2);
    QCOMPARE(vdata(r)->getWidth(), 3);
    QCOMPARE(vdata(r)->getHeight(), 2);
}

void tWorkflow::troi() {
    // This tests creation, editing, selection, and destruction
    // These could be done separately, but there's a fair amount of overlapping territory here
    loaddataset(r);

    // Drawing ROIs
    {
        // warning: fragile tests, something causing rounding errors here...
        makeroi_nonpoly(r, ROIVert::SHAPE::RECTANGLE, { 0.1, 0.1 }, { 3.1, 3.1 });
        QCOMPARE(rois(r)->size(), 1);
        QCOMPARE((*rois(r))[0].graphicsShape->getShapeType(), ROIVert::SHAPE::RECTANGLE);
        QCOMPARE((*rois(r))[0].graphicsShape->getVertices(), std::vector<QPoint>({ QPoint({0,0}), QPoint({3,3}) }));
        QCOMPARE(rois(r)->getSelected(), { 0 });

        makeroi_nonpoly(r, ROIVert::SHAPE::ELLIPSE, { 1, 1 }, { 4, 4 });
        QCOMPARE(rois(r)->size(), 2);
        QCOMPARE((*rois(r))[1].graphicsShape->getShapeType(), ROIVert::SHAPE::ELLIPSE);
        QCOMPARE((*rois(r))[1].graphicsShape->getVertices(), std::vector<QPoint>({ QPoint({1,1}), QPoint({4,4}) }));
        QCOMPARE(rois(r)->getSelected(), { 1 });

        makeroi_poly(r, { { 2.1, 2.1 }, { 5.1, 2.1 }, { 2.1, 5.1 } });
        QCOMPARE(rois(r)->size(), 3);
        QCOMPARE((*rois(r))[2].graphicsShape->getShapeType(), ROIVert::SHAPE::POLYGON);
        QCOMPARE((*rois(r))[2].graphicsShape->getVertices(), std::vector<QPoint>({ QPoint({2,2}), QPoint({5,2}), QPoint({2,5}) }));
        QCOMPARE(rois(r)->getSelected(), { 2 });
    }

    // Selecting ROIs
    {
        // This test focuses on selction mechanics but not precision, which is 
        // somewhat imperfect (i.e. you can select a target when not exactly on top of it).

        selctshapeaction(r, ROIVert::SHAPE::SELECT);
        update();

        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(.1, .1));
        QCOMPARE(rois(r)->getSelected(), { 0 });

        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(2.1, 2.1));
        QCOMPARE(rois(r)->getSelected(), { 2 });

        // shift to select
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, iview(r)->mapFromScene(.1, .1));
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>({ 0, 2 }));

        // no-op shift off roi
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, iview(r)->mapFromScene(5, .1));
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>({ 0, 2 }));

        // shift to de-select
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, iview(r)->mapFromScene(.1, .1));
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>({ 2 }));

        // (re-select)
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, iview(r)->mapFromScene(.1, .1));
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>({ 0, 2 }));

        // unselect
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(5, .1));
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>());

        // select two then click one without shift to select just that one
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(.1, .1));
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier, iview(r)->mapFromScene(2.1, 2.1));
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>({ 0, 2 }));
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(.1, .1));
        QCOMPARE(rois(r)->getSelected(), { 0 });

        // select all with key
        QTest::keyClick(iview(r)->viewport(), Qt::Key::Key_A, Qt::KeyboardModifier::ControlModifier);
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>({ 0, 1, 2 }));
    }

    // Selecting via Charts
    {
        // unselect all before proceeding
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(5, .1));
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>());

        dw::tview(r)->show();
        // click the first chart
        auto tc0 = (*rois(r))[0].Trace->getTraceChart();
        auto tc1 = (*rois(r))[1].Trace->getTraceChart();
        QTest::mousePress(tc0, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
        QCOMPARE(rois(r)->getSelected(), { 0 });
        QTest::mousePress(tc1, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
        QCOMPARE(rois(r)->getSelected(), { 1 });
        QTest::mousePress(tc0, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>({ 0, 1 }));

        // select all with key
        QTest::keyClick(dw::tview(r), Qt::Key::Key_A, Qt::KeyboardModifier::ControlModifier);
        QCOMPARE(rois(r)->getSelected(), std::vector<size_t>({ 0, 1, 2 }));

        dw::tview(r)->hide();
        update();
    }

    // Editing ROIs
    {
        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(.1, .1));
        QCOMPARE(rois(r)->getSelected(), { 0 });

        QTest::mousePress(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(3, 3));
        QTest::mouseMove(iview(r), iview(r)->mapFromScene(4, 4));
        pause(50);
        QTest::mouseRelease(iview(r)->viewport(), Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, iview(r)->mapFromScene(4, 4));
        pause(50);
        QCOMPARE((*rois(r))[0].graphicsShape->getVertices(), std::vector<QPoint>({ QPoint({4, 4}), QPoint({0, 0}) }));
    }
}

void tWorkflow::tzoom() {
    // I can't find a way to set the qApp KeyboardModifiers set with
    loaddataset(r);
    pause(1000);
    auto zoomer = iview(r)->findChild<ZoomPan*>();
    zoomer->setModifier(Qt::NoModifier);

    auto z1 = std::make_pair<double, double>(iview(r)->transform().m11(), iview(r)->transform().m22());

    auto evt = QWheelEvent({ 0, 0 }, 120, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    QApplication::sendEvent(iview(r)->viewport(), &evt); update();
    auto z2 = std::make_pair<double, double>(iview(r)->transform().m11(), iview(r)->transform().m22());

    evt = QWheelEvent({ 0, 0 }, 120, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    QApplication::sendEvent(iview(r)->viewport(), &evt); update();
    auto z3 = std::make_pair<double, double>(iview(r)->transform().m11(), iview(r)->transform().m22());

    // pan tests are too tempermental to include

    evt = QWheelEvent({ 0, 0 }, -120, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    QApplication::sendEvent(iview(r)->viewport(), &evt); update();
    QApplication::sendEvent(iview(r)->viewport(), &evt); update();
    QApplication::sendEvent(iview(r)->viewport(), &evt); update();

    auto z5 = std::make_pair<double, double>(iview(r)->transform().m11(), iview(r)->transform().m22());
    QVERIFY(z1.first < z2.first&& z1.second < z2.second);
    QVERIFY(z2.first < z3.first&& z2.second < z3.second);
    QCOMPARE(z5, z1);
}
//      Contrast/display settings
//      Change styling on rois, charts
//      import/export? or maybe this is well covered in unit
//      Play/pause video, change playspeed, change df/f
//      Delete Rois