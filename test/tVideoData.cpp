#include <QtTest/QtTest>
#include <QJsonDocument>

#include "tVideoData.h"
#include "VideoData.h"
#include "ROIVertEnums.h"

void tVideoData::init() {
    data = new VideoData;
    loadjsonexpected();
    loadmultipage(); // default load with no downsampling
}
void tVideoData::cleanup() {
    delete data;
    data = nullptr;
}

void tVideoData::loadmultipage(int dst, int dss) {
    QStringList f = { TEST_RESOURCE_DIR "/roiverttestdata.tiff" };
    QVERIFY(QFile(f[0]).exists());
    data->load(f, dst, dss, false);
}

void tVideoData::loadjsonexpected() {
    QFile file(TEST_RESOURCE_DIR "/roiverttestdata.json");
    auto isopen = file.open(QIODevice::ReadOnly);
    QVERIFY(isopen);

    QByteArray jdata = file.readAll();
    auto doc = QJsonDocument::fromJson(jdata);

    auto raw = doc.object()["raw"].toArray();
    int nframes(raw.size());
    int w = raw[0].toArray().size();
    int h = raw[0].toArray()[0].toArray().size();

    expraw.clear();
    expraw.resize(nframes);
    for (int f = 0; f < nframes; ++f) {
        expraw[f] = cv::Mat(h, w, CV_8UC1);
        for (int c = 0; c < w; ++c) {
            for (int r = 0; r < h; ++r) {
                expraw[f].at<uint8_t>(r, c) = raw[f].toArray()[c].toArray()[r].toInt();
            }
        }
    }

    auto dffi = doc.object()["dffi"].toArray();
    expdffi.clear();
    expdffi.resize(nframes);
    for (int f = 0; f < nframes; ++f) {
        expdffi[f] = cv::Mat(h, w, CV_8UC1);
        for (int c = 0; c < w; ++c) {
            for (int r = 0; r < h; ++r) {
                expdffi[f].at<uint8_t>(r, c) = dffi[f].toArray()[c].toArray()[r].toInt();
            }
        }
    }

    auto rawmean = doc.object()["rawmean"].toArray();
    auto rawmax = doc.object()["rawmax"].toArray();
    auto rawmin = doc.object()["rawmin"].toArray();
    auto dffmax = doc.object()["dffmax"].toArray();
    auto dffmin = doc.object()["dffmin"].toArray();
    expmeanraw = cv::Mat(h, w, CV_8UC1);
    expmaxraw = cv::Mat(h, w, CV_8UC1);
    expminraw = cv::Mat(h, w, CV_8UC1);
    expmaxdff = cv::Mat(h, w, CV_8UC1);
    expmindff = cv::Mat(h, w, CV_8UC1);
    for (int c = 0; c < w; ++c) {
        for (int r = 0; r < h; ++r) {
            expmeanraw.at<uint8_t>(r, c) = rawmean[c].toArray()[r].toInt();
            expmaxraw.at<uint8_t>(r, c) = rawmax[c].toArray()[r].toInt();
            expminraw.at<uint8_t>(r, c) = rawmin[c].toArray()[r].toInt();
            expmaxdff.at<uint8_t>(r, c) = dffmax[c].toArray()[r].toInt();
            expmindff.at<uint8_t>(r, c) = dffmin[c].toArray()[r].toInt();
        }
    }

    auto jroiwf = doc.object()["roi_wholefield"].toArray();
    for (const auto& val : jroiwf) {
        roiwholefield.push_back(static_cast<float>(val.toDouble()));
    }

    auto jroiel = doc.object()["roi_wholeellipse"].toArray();
    for (const auto& val : jroiel) {
        roiellipse.push_back(static_cast<float>(val.toDouble()));
    }

    auto jroitril = doc.object()["roi_tril"].toArray();
    for (const auto& val : jroitril) {
        roitril.push_back(static_cast<float>(val.toDouble()));
    }
}

void tVideoData::tload_data() {
    // data is just frame number...that gives a way to avoid a loop in the test and have debug information on a fail

    QTest::addColumn<int>("frame");
    for (int i = 0; i < 7; ++i) {
        QTest::newRow(std::to_string(i).c_str()) << i;
    }
}

void tVideoData::tload() {
    // if this test needs debugging, consider moving to the less performant but more debuggable strategy used for tdown*
    QCOMPARE(data->getNFrames(), 7);
    QCOMPARE(data->getdsSpace(), 1);
    QCOMPARE(data->getdsTime(), 1);
    QCOMPARE(data->getHeight(), 5);
    QCOMPARE(data->getWidth(), 6);
    QFETCH(int, frame);

    double minval, maxval;

    cv::minMaxLoc(data->get(false, 0, frame) - expraw[frame], &minval, &maxval);
    QCOMPARE(maxval, 0);

    cv::minMaxLoc(data->get(true, 0, frame) - expdffi[frame], &minval, &maxval);
    QVERIFY(maxval <= 1); // allow off-by-one errors, due to rounding (?)
}

void tVideoData::tproj() {
    double minval, maxval;
    auto actproj = data->get(false, (int)VideoData::projection::MEAN + 1, 0); // This interface is somewhat unclear!
    cv::minMaxLoc(actproj - expmeanraw, &minval, &maxval);
    QCOMPARE(maxval, 0);

    actproj = data->get(false, (int)VideoData::projection::MIN + 1, 0);
    cv::minMaxLoc(actproj - expminraw, &minval, &maxval);
    QCOMPARE(maxval, 0);

    actproj = data->get(false, (int)VideoData::projection::MAX + 1, 0);
    cv::minMaxLoc(actproj - expmaxraw, &minval, &maxval);
    QCOMPARE(maxval, 0);

    actproj = data->get(true, (int)VideoData::projection::MIN + 1, 0);
    cv::minMaxLoc(actproj - expmindff, &minval, &maxval);
    QVERIFY(maxval <= 1);

    actproj = data->get(true, (int)VideoData::projection::MAX + 1, 0);
    cv::minMaxLoc(actproj - expmaxdff, &minval, &maxval);
    QVERIFY(maxval <= 1);
}

void tVideoData::tdowns_data() {
    // data is just frame number...that gives a way to avoid a loop in the test and have debug information on a fail
    QTest::addColumn<int>("frame");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("col");
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                const auto name = "fr:" + std::to_string(i) + ", row:" + std::to_string(j) + ", col:" + std::to_string(k);
                QTest::newRow(name.c_str()) << i << j << k;
            }
        }
    }
}

void tVideoData::tdowns() {
    loadmultipage(1, 2);

    QCOMPARE(data->getNFrames(), 7);
    QCOMPARE(data->getdsSpace(), 2);
    QCOMPARE(data->getdsTime(), 1);
    QCOMPARE(data->getHeight(), 2);
    QCOMPARE(data->getWidth(), 3);

    QFETCH(int, frame);
    QFETCH(int, row);
    QFETCH(int, col);

    auto m = data->get(false, 0, frame);
    QCOMPARE(m.at<uint8_t>(row, col), expraw[frame].at<uint8_t>(row * 2, col * 2));
}

void tVideoData::tdownt_data() {
    // data is just frame number...that gives a way to avoid a loop in the test and have debug information on a fail
    QTest::addColumn<int>("frame");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("col");
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                const auto name = "fr:" + std::to_string(i) + ", row:" + std::to_string(j) + ", col:" + std::to_string(k);
                QTest::newRow(name.c_str()) << i << j << k;
            }
        }
    }
}

void tVideoData::tdownt() {
    loadmultipage(2, 1);
    QCOMPARE(data->getNFrames(), 3);
    QCOMPARE(data->getdsSpace(), 1);
    QCOMPARE(data->getdsTime(), 2);
    QCOMPARE(data->getHeight(), 5);
    QCOMPARE(data->getWidth(), 6);

    QFETCH(int, frame);
    QFETCH(int, row);
    QFETCH(int, col);
    auto m = data->get(false, 0, frame);
    QCOMPARE(m.at<uint8_t>(row, col), expraw[frame * 2 + 1].at<uint8_t>(row, col));
}

void tVideoData::tfr() {
    data->setFrameRate(14);
    QCOMPARE(data->getTMax(), .5);
}

void tVideoData::ttrace() {
    {
        auto trace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, QRect(0, 0, 7, 6), { QPoint(0,0), QPoint(6,5) });
        for (size_t i = 0; i < trace.size().width; ++i) {
            QVERIFY(std::abs(trace.at<float>(0, i) - roiwholefield[i]) < .001);
        }
    }
    {
        // Testing ellipse with such a small size doesn't make much sense. This ended up cropping out two points (top and bottom left corners)
        // Not sure if there's a pixel offset issue wherein we should also be cropping out the right side?
        auto trace = data->computeTrace(ROIVert::SHAPE::ELLIPSE, QRect(0, 0, 7, 6), { QPoint(0,0), QPoint(6,5) });
        for (size_t i = 0; i < trace.size().width; ++i) {
            QVERIFY(std::abs(trace.at<float>(0, i) - roiellipse[i]) < .001);
        }
    }
    {
        auto trace = data->computeTrace(ROIVert::SHAPE::POLYGON, QRect(0, 0, 7, 6), { QPoint(0,0), QPoint(0,5), QPoint(5,5) });
        for (size_t i = 0; i < trace.size().width; ++i) {
            QVERIFY(std::abs(trace.at<float>(0, i) - roitril[i]) < .001);
        }
    }
}

void tVideoData::tdeadpixel() {
    QStringList f = { TEST_RESOURCE_DIR "/roiverttestdata_deadpix.tiff" };
    data->load(f, 1, 1, false);
    auto m = data->get(true, 0, 0);
    auto trace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, QRect(0, 0, 7, 6), { QPoint(0,0), QPoint(6,5) });
    for (size_t i = 0; i < trace.size().width; ++i) {
        QVERIFY(!isnan(trace.at<float>(0, i)));
    }
}

void tVideoData::tmultifile_data() {
    QTest::addColumn<int>("frame");
    for (int i = 0; i < 7; ++i) {
        QTest::newRow(std::to_string(i).c_str()) << i;
    }
}

void tVideoData::tmultifile() {
    QStringList f;
    for (size_t i = 0; i < 7; ++i) {
        f << (TEST_RESOURCE_DIR "/roiverttestdata_mf" + std::to_string(i + 1) + ".tiff").c_str();
    }
    data->load(f, 1, 1, true);

    QCOMPARE(data->getNFrames(), 7);
    QCOMPARE(data->getdsSpace(), 1);
    QCOMPARE(data->getdsTime(), 1);
    QCOMPARE(data->getHeight(), 5);
    QCOMPARE(data->getWidth(), 6);
    QFETCH(int, frame);

    double minval, maxval;
    cv::minMaxLoc(data->get(false, 0, frame) - expraw[frame], &minval, &maxval);
    QCOMPARE(maxval, 0);

    cv::minMaxLoc(data->get(true, 0, frame) - expdffi[frame], &minval, &maxval);
    QVERIFY(maxval <= 1); // allow off-by-one errors, due to rounding (?)
}

void tVideoData::temptyfilelist() {
    // an empty file list is currently a no-op, i.e. all data unchanged

    QStringList f;
    data->load(f, 1, 1, true);

    QCOMPARE(data->getNFrames(), 7);
    QCOMPARE(data->getdsSpace(), 1);
    QCOMPARE(data->getdsTime(), 1);
    QCOMPARE(data->getHeight(), 5);
    QCOMPARE(data->getWidth(), 6);

    auto m = data->get(false, 0, 0);
    QCOMPARE(m.size().height, 5);
    QCOMPARE(m.size().width, 6);
}

void tVideoData::tgetoverflow() {
    auto m = data->get(false, 0, 100);
    QCOMPARE(m.size().height, 0);
    QCOMPARE(m.size().width, 0);
}

void tVideoData::tnowidthtrace() {
    auto trace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, QRect(0, 0, 0, 0), { QPoint(0,0), QPoint(0,0) });
    for (size_t i = 0; i < 7; ++i) {
        QCOMPARE(trace.at<float>(i), 0);
    }
}

void tVideoData::thistogram() {
    std::vector<float> histogram(255);
    data->getHistogram(true, histogram);
#ifdef NDEBUG
    QCOMPARE(histogram.size(), 256);
    QCOMPARE(histogram[0], 3);
    auto histsum = std::accumulate(histogram.begin(), histogram.end(), 0);
    QCOMPARE(histsum, 210);
#else
    QEXPECT_FAIL("", "opencv histograms fail sporadically in DEBUG builds", Abort);
    QCOMPARE(1, 2);
#endif // NDEBUG
}