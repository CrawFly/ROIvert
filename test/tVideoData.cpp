#include <QtTest/QtTest>
#include <QJsonDocument>

#include "tVideoData.h"
#include "VideoData.h"
#include "ROIVertEnums.h"
#include "testUtils.h"

namespace {
    // ** a little matrix type would help clean this all up!
    // ... just make it a flat vector
    // ... method to get a projection
    // ... method to find grand max/min
    // ... method to convert between double and uint8_t
    // ... method to convert to vector vector vector
    // 
    ROIVertMat3D<uint8_t> getExpectedRaw(size_t nframes, size_t height, size_t width) {
        std::vector<uint8_t> vecExpectedRawData(nframes * height * width);
        std::iota(vecExpectedRawData.begin(), vecExpectedRawData.end(), 1);
        return ROIVertMat3D<uint8_t>(vecExpectedRawData, nframes, height, width);
    }
    ROIVertMat3D<uint8_t> getExpectedDff(size_t nframes, size_t height, size_t width) {
        auto raw = getExpectedRaw(nframes, height, width);
        auto mu = raw.getMeanProjection().cast<double>();
        auto expectedDff = (raw.cast<double>() - mu) / mu;
        auto dffmin = expectedDff.globalMin(), dffmax = expectedDff.globalMax();
        auto expectedDffNative = (((expectedDff - dffmin) / (dffmax - dffmin)) * 255.).cast<uint8_t>();
        return expectedDffNative;
    }
}


void tVideoData::init() {
    data = new VideoData;
}
void tVideoData::cleanup() {
    delete data;
    data = nullptr;
}

void tVideoData::tload_data() {
    QTest::addColumn<int>("dstype_int");
    QTest::newRow("ONESTACK") << static_cast<int>(datasettype::ONESTACK);
    QTest::newRow("SINGLEFRAMES") << static_cast<int>(datasettype::SINGLEFRAMES);
    QTest::newRow("MULTIPLESTACKS") << static_cast<int>(datasettype::MULTIPLESTACKS);
}

void tVideoData::tload() {
    QFETCH(int, dstype_int);
    auto dstype = static_cast<datasettype>(dstype_int);
    loaddataset(data, dstype);
    QCOMPARE(data->getNFrames(), 8);
    QCOMPARE(data->getdsSpace(), 1);
    QCOMPARE(data->getdsTime(), 1);
    QCOMPARE(data->getHeight(), 5);
    QCOMPARE(data->getWidth(), 6);


    std::vector<cv::Mat> cvRawData(data->getNFrames());
    std::vector<cv::Mat> cvDffData(data->getNFrames());
    for (size_t i = 0; i < data->getNFrames(); ++i) {
        cvRawData[i] = data->get(false, 0, i);
        cvDffData[i] = data->get(true, 0, i);
        QCOMPARE(cvRawData[i].size().height, data->getHeight());
        QCOMPARE(cvRawData[i].size().width, data->getWidth());
        QCOMPARE(cvDffData[i].size().height, data->getHeight());
        QCOMPARE(cvDffData[i].size().width, data->getWidth());
    }
    auto RawData = ROIVertMat3D<uint8_t>(cvRawData);
    auto ExpectedRaw = getExpectedRaw(data->getNFrames(), data->getHeight(), data->getWidth());
    QCOMPARE(RawData, ExpectedRaw);

    // compute expected df/f
    auto DffData = ROIVertMat3D<uint8_t>(cvDffData);
    auto ExpectedDff = getExpectedDff(data->getNFrames(), data->getHeight(), data->getWidth());
    bool issame = ROIVertMat3D<uint8_t>::almostequal(DffData, ExpectedDff, 2);
    QVERIFY(issame);
}



void tVideoData::tproj() {
    loaddataset(data);
    auto expraw = getExpectedRaw(data->getNFrames(), data->getHeight(), data->getWidth());

    auto cvActRawMinProj = data->get(false, static_cast<int>(VideoData::projection::MIN)+1, 0);
    ROIVertMat3D<uint8_t> ActRawMinProj(std::vector<cv::Mat>({ cvActRawMinProj }));
    auto ExpRawMinProj = expraw.getMinProjection();
    QCOMPARE(ActRawMinProj, ExpRawMinProj);

    auto cvActRawMaxProj = data->get(false, static_cast<int>(VideoData::projection::MAX)+1, 0);
    ROIVertMat3D<uint8_t> ActRawMaxProj(std::vector<cv::Mat>({ cvActRawMaxProj }));
    auto ExpRawMaxProj = expraw.getMaxProjection();
    QCOMPARE(ActRawMaxProj, ExpRawMaxProj);

    auto cvActRawMeanProj = data->get(false, static_cast<int>(VideoData::projection::MEAN)+1, 0);
    ROIVertMat3D<uint8_t> ActRawMeanProj(std::vector<cv::Mat>({ cvActRawMeanProj }));
    auto ExpRawMeanProj = expraw.getMeanProjection();
    QCOMPARE(ActRawMeanProj, ExpRawMeanProj);

    // todo: sumproj
    // todo: dff
}

/*
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
*/