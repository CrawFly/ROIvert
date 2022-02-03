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

    ROIVertMat3D<uint8_t> getExpectedRaw(VideoData* data) {
        return getExpectedRaw(data->getNFrames(), data->getHeight(), data->getWidth());
    }
    
    ROIVertMat3D<double> getExpectedDff_double(size_t nframes, size_t height, size_t width) {
        auto raw = getExpectedRaw(nframes, height, width);
        auto mu = raw.getMeanProjection().cast<double>();
        auto expectedDff = (raw.cast<double>() - mu) / mu;
        return expectedDff;
    }
    ROIVertMat3D<double> getExpectedDff_double(VideoData* data) {
        return getExpectedDff_double(data->getNFrames(), data->getHeight(), data->getWidth());
    }
    ROIVertMat3D<uint8_t> getExpectedDff(size_t nframes, size_t height, size_t width) {
        auto expectedDff = getExpectedDff_double(nframes, height, width);
        auto dffmin = expectedDff.globalMin(), dffmax = expectedDff.globalMax();
        auto expectedDffNative = (((expectedDff - dffmin) / (dffmax - dffmin)) * 255.).cast<uint8_t>();
        return expectedDffNative;
    }
    ROIVertMat3D<uint8_t> getExpectedDff(VideoData* data) {
        return getExpectedDff(data->getNFrames(), data->getHeight(), data->getWidth());
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
    auto ExpectedRaw = getExpectedRaw(data);
    QCOMPARE(RawData, ExpectedRaw);

    // compute expected df/f
    auto DffData = ROIVertMat3D<uint8_t>(cvDffData);
    auto ExpectedDff = getExpectedDff(data);
    bool issame = ROIVertMat3D<uint8_t>::almostequal(DffData, ExpectedDff, 2);
    QVERIFY(issame);
}

void tVideoData::tproj_raw() {
    loaddataset(data);
    auto exp = getExpectedRaw(data);

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
    auto exp = getExpectedDff(data);

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

void tVideoData::tfr() {
    loaddataset(data);
    QCOMPARE(data->getNFrames(), 8);
    data->setFrameRate(8);
    QCOMPARE(data->getTMax(), 1.);
    data->setFrameRate(4);
    QCOMPARE(data->getTMax(), 2.);
    data->setFrameRate(16);
    QCOMPARE(data->getTMax(), .5);
}

void tVideoData::ttrace() {
    loaddataset(data);
        auto dff = getExpectedDff_double(data);
    
    { // One Pixel ROI:
        auto trace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, QRect(0, 0, 2, 2), { QPoint(0, 0), QPoint(2, 2) });
        ROIVertMat3D<float> tracemat(std::vector<cv::Mat>({ trace }));
        auto Act = tracemat.getAsVectors()[0][0];
        auto Exp = dff.getPixel(0,0);
        QVERIFY(nearlyequal(Act, Exp));
    }
    { // 1x2 ROI:
        auto trace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, QRect(0, 0, 3, 2), { QPoint(0, 0), QPoint(3, 2) });
        ROIVertMat3D<float> tracemat(std::vector<cv::Mat>({ trace }));
        auto Act = tracemat.getAsVectors()[0][0];
        
        auto Exp1 = dff.getPixel(0,0);
        auto Exp2 = dff.getPixel(0,1);
        std::vector<double> Exp(Exp1.size());
        for (size_t i = 0; i < Exp.size(); ++i) {
            Exp[i] = (Exp1[i] + Exp2[i])/2.;
        }
        QVERIFY(nearlyequal(Act, Exp));
    }
    { // WholeField:
        auto trace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, QRect(0, 0, 7, 6), { QPoint(0, 0), QPoint(7, 6) });
        ROIVertMat3D<float> tracemat(std::vector<cv::Mat>({ trace }));
        auto Act = tracemat.getAsVectors()[0][0];

        std::vector<double> Exp(data->getNFrames(), 0);
        for (size_t i = 0; i < data->getHeight(); ++i) {
            for (size_t j = 0; j < data->getWidth(); ++j) {
                auto pixel = dff.getPixel(i, j);
                for (size_t f = 0; f < pixel.size(); ++f) {
                    Exp[f] += pixel[f];
                }
            }
        }
        for (auto& val : Exp) {
            val /= (data->getWidth() * data->getHeight());
        }
        QVERIFY(nearlyequal(Act, Exp));
    }
    { // WholeField Ellipse: (note that ellipses are weird when things are so small)
        auto trace = data->computeTrace(ROIVert::SHAPE::ELLIPSE, QRect(0, 0, 6, 5), { QPoint(0, 0), QPoint(6, 5) });
        ROIVertMat3D<float> tracemat(std::vector<cv::Mat>({ trace }));
        auto Act = tracemat.getAsVectors()[0][0];
        std::vector<double> Exp(data->getNFrames(), 0);

        std::vector<std::pair<size_t, size_t>> pixlist = {
                          {0,2},
                   {1,1}, {1,2}, {1,3},
            {2,0}, {2,1}, {2,2}, {2,3}, {2,4},
                   {3,1}, {3,2}, {3,3}
        };

        for (const auto& [row, col] : pixlist) {
            auto pixel = dff.getPixel(row, col);
            for (size_t f = 0; f < pixel.size(); ++f) {
                Exp[f] += pixel[f];
            }
        }

        for (auto& val : Exp) {
            val /= pixlist.size();
        }
        
        QVERIFY(nearlyequal(Act, Exp));
    }

    {
        auto trace = data->computeTrace(ROIVert::SHAPE::POLYGON, QRect(0, 0, 7, 6), { QPoint(0, 0), QPoint(7, 6), QPoint(0, 6)});
        
        ROIVertMat3D<float> tracemat(std::vector<cv::Mat>({ trace }));
        auto Act = tracemat.getAsVectors()[0][0];

        std::vector<double> Exp(data->getNFrames(), 0);

        std::vector<std::pair<size_t, size_t>> pixlist = {
            {0,0},
            {1,0}, {1,1},
            {2,0}, {2,1}, {2,2}, {2,3},
            {3,0}, {3,1}, {3,2}, {3,3}, {3,4},
            {4,0}, {4,1}, {4,2}, {4,3}, {4,4}, {4,5}
        };

        for (const auto& [row, col] : pixlist) {
            auto pixel = dff.getPixel(row, col);
            for (size_t f = 0; f < pixel.size(); ++f) {
                Exp[f] += pixel[f];
            }
        }

        for (auto& val : Exp) {
            val /= pixlist.size();
        }
        QVERIFY(nearlyequal(Act, Exp));
    }
}

void tVideoData::tdeadpixel() {
    // This test guards against a bug where a dead pixel killed traces (due to div0)
    loaddataset(data, datasettype::DEADPIX);
    auto dff_frame0 = ROIVertMat3D<uint8_t>(std::vector<cv::Mat>{ data->get(true, 0, 1)} );
    QCOMPARE(dff_frame0.getPixel(0, 0)[0], 0);
    QCOMPARE(dff_frame0.getPixel(0, 1)[0], 37);
    QCOMPARE(dff_frame0.getPixel(0, 2)[0], 0);

    auto trace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, QRect(0, 0, 7, 6), { QPoint(0, 0), QPoint(7, 6) });
    ROIVertMat3D<float> tracemat(std::vector<cv::Mat>({ trace }));
    auto Act = tracemat.getAsVectors()[0][0];
    auto dff = getExpectedDff_double(data);
    std::vector<double> Exp(data->getNFrames(), 0);

    
    std::vector<std::pair<size_t, size_t>> pixlist = {
               {0,1},        {0,3},        {0,5},
        {1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5},
               {2,1},        {2,3},        {2,5},
        {3,0}, {3,1}, {3,2}, {3,3}, {3,4}, {3,5},
               {4,1},        {4,3},        {4,5}
    };

    for (const auto& [row, col] : pixlist) {
        auto pixel = dff.getPixel(row, col);
        for (size_t f = 0; f < pixel.size(); ++f) {
            Exp[f] += pixel[f];
        }
    }

    for (auto& val : Exp) {
        val /= pixlist.size();
    }
    QVERIFY(nearlyequal(Act, Exp));
}
void tVideoData::tgetoverflow() {
    loaddataset(data);
    auto m = data->get(false, 0, 100);
    QCOMPARE(m.size().height, 0);
    QCOMPARE(m.size().width, 0);
}
void tVideoData::tnowidthtrace() {
    loaddataset(data);
    auto trace = data->computeTrace(ROIVert::SHAPE::RECTANGLE, QRect(0, 0, 0, 0), { QPoint(0,0), QPoint(0,0) });
    for (size_t i = 0; i < 7; ++i) {
        QCOMPARE(trace.at<float>(i), 0);
    }
}

void tVideoData::thistogram() {
    loaddataset(data);
    std::vector<float> histogram(255);
    data->getHistogram(true, histogram);
#ifdef NDEBUG
    QCOMPARE(histogram.size(), 256);
    QCOMPARE(histogram[0], 2);
    auto histsum = std::accumulate(histogram.begin(), histogram.end(), 0);
    QCOMPARE(histsum, 240);
#else
    QEXPECT_FAIL("", "opencv histograms fail sporadically in DEBUG builds", Abort);
    QCOMPARE(1, 2);
#endif // NDEBUG
}