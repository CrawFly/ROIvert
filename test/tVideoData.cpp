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

void tVideoData::tproj_raw() {
    loaddataset(data);
    auto exp = getExpectedRaw(data->getNFrames(), data->getHeight(), data->getWidth());

    auto cvActMin = data->get(false, static_cast<int>(VideoData::projection::MIN)+1, 0);
    ROIVertMat3D<uint8_t> ActMin(std::vector<cv::Mat>({ cvActMin }));
    auto ExpMin = exp.getMinProjection();
    QCOMPARE(ActMin, ExpMin);

    auto cvActMax = data->get(false, static_cast<int>(VideoData::projection::MAX)+1, 0);
    ROIVertMat3D<uint8_t> ActMax(std::vector<cv::Mat>({ cvActMax }));
    auto ExpMax = exp.getMaxProjection();
    QCOMPARE(ActMax, ExpMax);

    auto cvActMean = data->get(false, static_cast<int>(VideoData::projection::MEAN)+1, 0);
    ROIVertMat3D<uint8_t> ActMean(std::vector<cv::Mat>({ cvActMean }));
    auto ExpMean = exp.getMeanProjection();
    QCOMPARE(ActMean, ExpMean);
}
void tVideoData::tproj_dff() {
    loaddataset(data);
    auto exp = getExpectedDff(data->getNFrames(), data->getHeight(), data->getWidth());

    auto cvActMin = data->get(true, static_cast<int>(VideoData::projection::MIN)+1, 0);
    ROIVertMat3D<uint8_t> ActMin(std::vector<cv::Mat>({ cvActMin }));
    auto ExpMin = exp.getMinProjection();
    QVERIFY(ROIVertMat3D<uint8_t>::almostequal(ActMin, ExpMin, 2));
    

    auto cvActMax = data->get(true, static_cast<int>(VideoData::projection::MAX)+1, 0);
    ROIVertMat3D<uint8_t> ActMax(std::vector<cv::Mat>({ cvActMax }));
    auto ExpMax = exp.getMaxProjection();
    QVERIFY(ROIVertMat3D<uint8_t>::almostequal(ActMax, ExpMax, 2));
    
    auto cvActMean = data->get(true, static_cast<int>(VideoData::projection::MEAN)+1, 0);
    ROIVertMat3D<uint8_t> ActMean(std::vector<cv::Mat>({ cvActMean }));
    auto cvExpMean = cv::Mat::zeros(cvActMean.size(), cvActMean.type());
    ROIVertMat3D<uint8_t> ExpMean(std::vector<cv::Mat>({ cvExpMean }));
    QCOMPARE(ActMean, ExpMean);
}

void tVideoData::tdowns_data() {
    QTest::addColumn<int>("dstype_int");
    QTest::newRow("ONESTACK") << static_cast<int>(datasettype::ONESTACK);
    QTest::newRow("SINGLEFRAMES") << static_cast<int>(datasettype::SINGLEFRAMES);
    QTest::newRow("MULTIPLESTACKS") << static_cast<int>(datasettype::MULTIPLESTACKS);
}

void tVideoData::tdowns() {
    QFETCH(int, dstype_int);
    auto dstype = static_cast<datasettype>(dstype_int);
    loaddataset(data, dstype, 10., 2);
    
    QCOMPARE(data->getNFrames(), 8);
    QCOMPARE(data->getdsSpace(), 2);
    QCOMPARE(data->getdsTime(), 1);
    QCOMPARE(data->getHeight(), 2);
    QCOMPARE(data->getWidth(), 3);

    {
        auto cvActData = data->get(false, 0, 0);
        auto ActData = ROIVertMat3D<uint8_t>(std::vector<cv::Mat>{ cvActData });
        auto ExpData = ROIVertMat3D<uint8_t>({ 1, 3, 5, 13, 15, 17 }, 1, 2, 3);
        QCOMPARE(ActData, ExpData);
    }    
    {
        auto cvActData = data->get(false, 0, 7);
        auto ActData = ROIVertMat3D<uint8_t>(std::vector<cv::Mat>{ cvActData });
        auto ExpData = ROIVertMat3D<uint8_t>({ 211, 213, 215, 223, 225, 227 }, 1, 2, 3);
        QCOMPARE(ActData, ExpData);
    }
    {
        auto cvActData = data->get(true, 0, 7);
        auto ActData = ROIVertMat3D<uint8_t>(std::vector<cv::Mat>{ cvActData });
        auto ExpData = ROIVertMat3D<uint8_t>({ 255, 254, 251, 243, 241, 239 }, 1, 2, 3);
        QCOMPARE(ActData, ExpData);
    }
}

void tVideoData::tdownt_data() {
    QTest::addColumn<int>("dstype_int");
    QTest::newRow("ONESTACK") << static_cast<int>(datasettype::ONESTACK);
    QTest::newRow("SINGLEFRAMES") << static_cast<int>(datasettype::SINGLEFRAMES);
    QTest::newRow("MULTIPLESTACKS") << static_cast<int>(datasettype::MULTIPLESTACKS);
}

void tVideoData::tdownt() {
    QFETCH(int, dstype_int);
    auto dstype = static_cast<datasettype>(dstype_int);
    loaddataset(data, dstype, 10., 1, 2);
    
    QCOMPARE(data->getNFrames(), 4);
    QCOMPARE(data->getdsSpace(), 1);
    QCOMPARE(data->getdsTime(), 2);
    QCOMPARE(data->getHeight(), 5);
    QCOMPARE(data->getWidth(), 6);


    auto expraw = getExpectedRaw(8, 5, 6);
    
    for (size_t i = 0; i < 4; ++i) {
        auto cvActData = data->get(false, 0, i);
        auto ActData = ROIVertMat3D<uint8_t>(std::vector<cv::Mat>{ cvActData });
        auto ExpData = expraw.getSlice(i*2);
        QCOMPARE(ActData, ExpData);
    }
}




/*

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