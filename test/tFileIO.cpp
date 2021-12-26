#include "tFileIO.h"

#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QFile>

#include "FileIO.h"
#include "ROI/ROIs.h"
#include "ImageView.h"
#include "dockwidgets/TraceViewWidget.h"
#include "VideoData.h"

class filescopeguard {
public:
    filescopeguard(QFile* f) : file(f) { }
    ~filescopeguard() { file->remove(); }
private:
    QFile* file;
};

struct tFileIO::objptrs {
    ImageView iview;
    TraceViewWidget tview;
    VideoData vdata;
    ROIs* rois = nullptr;

    objptrs() {
        QStringList f = { TEST_RESOURCE_DIR "/roiverttestdata.tiff" };
        vdata.load(f, 1, 1, false);
        vdata.setFrameRate(1);

        rois = new ROIs(&iview, &tview, &vdata);

        // push 3 rois:
        rois->pushROI(QPoint(0, 0), ROIVert::SHAPE::RECTANGLE);
        rois->pushROI(QPoint(0, 0), ROIVert::SHAPE::RECTANGLE);
        rois->pushROI(QPoint(0, 0), ROIVert::SHAPE::RECTANGLE);

        {
            auto& shp = (*rois)[0].graphicsShape;
            shp->setVertices({ QPoint(1,1), QPoint(3,3) });
            (*rois)[0].Trace->updateTrace(ROIVert::SHAPE::RECTANGLE, shp->getTightBoundingBox(), shp->getVertices());
        }
        {
            auto& shp = (*rois)[1].graphicsShape;
            shp->setVertices({ QPoint(2,2), QPoint(4,3) });
            (*rois)[1].Trace->updateTrace(ROIVert::SHAPE::RECTANGLE, shp->getTightBoundingBox(), shp->getVertices());
        }
        {
            auto& shp = (*rois)[2].graphicsShape;
            shp->setVertices({ QPoint(1,2), QPoint(3,4) });
            (*rois)[2].Trace->updateTrace(ROIVert::SHAPE::RECTANGLE, shp->getTightBoundingBox(), shp->getVertices());
        }

        rois->updateROITraces();
    }
};

tFileIO::tFileIO() : ptrs(std::make_unique<objptrs>()) { }

void tFileIO::init() {
    fileio = new FileIO(ptrs->rois, &ptrs->tview, &ptrs->vdata);
}
void tFileIO::cleanup() {
    delete fileio;
    fileio = nullptr;
}

void tFileIO::texporttraces() {
    fileio->exportTraces("traces.csv", false, false);
    QFile file("traces.csv");
    QVERIFY(file.exists());
    filescopeguard fsg(&file);

    // read the file
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray actstring = file.readAll();
    QString expstring;
    size_t nframes = ptrs->vdata.getNFrames();

    for (size_t f = 0; f < nframes; ++f) {
        for (size_t r = 0; r < 3; ++r) {
            auto val = (*ptrs->rois)[r].Trace->getTrace()[f];
            expstring += QString::number(val) + ",";
        }
        expstring.remove(expstring.length() - 1, 1);
        expstring += "\n";
    }
    QCOMPARE(actstring, expstring);
    file.close();

    fileio->exportTraces("traces.csv", true, false);
    QVERIFY(file.open(QFile::ReadOnly));
    actstring = file.readAll();
    QVERIFY(actstring.startsWith("\"ROI 1\",\"ROI 2\",\"ROI 3\"\n"));
    file.close();

    fileio->exportTraces("traces.csv", true, true);
    QVERIFY(file.open(QFile::ReadOnly));
    actstring = file.readAll();
    QVERIFY(actstring.startsWith("\"Time\",\"ROI 1\",\"ROI 2\",\"ROI 3\"\n"));

    auto splitstring = actstring.split('\n');
    QCOMPARE(splitstring.length(), 9); /// header + 7 frames + newline at end

    for (size_t i = 0; i < 7; ++i) {
        auto splitsplit = splitstring[i + 1].split(',');
        QVERIFY(!splitsplit.isEmpty());
        QCOMPARE(splitsplit[0], QString::number(i));
    }
}
void tFileIO::texportrois() {
    fileio->exportROIs("rois.json");
    QFile file("rois.json");
    QVERIFY(file.exists());
    filescopeguard fsg(&file);

    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray jdata = file.readAll();
    file.close();

    auto jdoc = QJsonDocument::fromJson(jdata);
    QVERIFY(jdoc.isObject());
    QVERIFY(jdoc.object().contains("ROIs"));
    QVERIFY(jdoc.object().contains("version"));

    auto jversion = jdoc.object()["version"];
    QVERIFY(jversion.isString());
    QCOMPARE(jversion.toString(), ROIVERTVERSION);

    auto jrois = jdoc.object()["ROIs"];
    QVERIFY(jrois.isArray());
    QCOMPARE(jrois.toArray().size(), 3);
    
    auto jroi = jdoc.object()["ROIs"].toArray()[0];
    QVERIFY(jroi.isObject());
    QVERIFY(jroi.toObject().contains("shape"));
    QVERIFY(jroi.toObject()["shape"].isObject());

    auto jroishape = jroi.toObject()["shape"].toObject();;
    QVERIFY(jroishape.contains("RGB"));
    QVERIFY(jroishape.contains("type"));
    QVERIFY(jroishape.contains("verts"));

    QVERIFY(jroishape["RGB"].isArray());
    QVERIFY(jroishape["RGB"].toArray()[0].isDouble());
    QCOMPARE(jroishape["RGB"].toArray().size(), 3);

    QVERIFY(jroishape["type"].isDouble());
    QCOMPARE(jroishape["type"].toDouble(), 0);

    QVERIFY(jroishape["verts"].isArray());
    QCOMPARE(jroishape["verts"].toArray().size(), 2);
}
void tFileIO::timportrois() {
    fileio->exportROIs("rois.json");
    QFile file("rois.json");
    QVERIFY(file.exists());
    filescopeguard fsg(&file);

    auto rois2 = std::make_unique<ROIs>(&ptrs->iview, &ptrs->tview, &ptrs->vdata);
    auto fileio2 = std::make_unique<FileIO>(rois2.get(), &ptrs->tview, &ptrs->vdata);
    fileio2->importROIs("rois.json");

    QCOMPARE(rois2->size(), 3);

    auto& shp = (*rois2)[0].graphicsShape;
    (*rois2)[0].Trace->updateTrace(ROIVert::SHAPE::RECTANGLE, shp->getTightBoundingBox(), shp->getVertices());
    auto t1 = (*rois2)[0].Trace->getTrace();
    QCOMPARE(t1.size(), 7);
    QVERIFY(std::abs(t1[0] - 0.273959) < .00001);
}

void tFileIO::texportcharts_data() {
    QTest::addColumn<QString>("ext");
    QTest::newRow("png") << "png";
    QTest::newRow("jpg") << "jpg";
}

void tFileIO::texportcharts() {
    QFETCH(QString, ext);
    fileio->exportCharts(QString("charts.%1").arg(ext), 500, 600, 100, false);
    for (size_t i = 0; i < 3; ++i)
    {
        auto fn = QString("charts_%1.%2").arg(i + 1).arg(ext);
        QFile file(fn);
        QVERIFY(file.exists());
        filescopeguard fsg(&file);
        QImage im(fn);
        QCOMPARE(im.size().width(), 500);
        QCOMPARE(im.size().height(), 600);
    }

    fileio->exportCharts(QString("chart.%1").arg(ext), 500, 600, 100, true);
    auto fn = QString("chart.%1").arg(ext);
    QFile file(fn);
    QVERIFY(file.exists());
    filescopeguard fsg(&file);
    QImage im(fn);
    QCOMPARE(im.size().width(), 500);
    QCOMPARE(im.size().height(), 600);
}