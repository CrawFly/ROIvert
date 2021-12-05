#include <QtTest/QtTest>
#include <QJsonDocument>

#include "tVideoData.h"
#include "VideoData.h"


void tVideoData::init() {
    data = new VideoData;
}
void tVideoData::cleanup() {
    delete data;
    data = nullptr;
}

void tVideoData::loadmultipage(int dst, int dss) {
    QStringList f = { "roiverttestdata.tiff" };
    data->load(f, dst, dss, false);
}

void tVideoData::loadjsonexpected() {
    QFile file("roiverttestdata.json");
    assert(file.open(QIODevice::ReadOnly));

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
            //qDebug() << rawmean[c];
            expmeanraw.at<uint8_t>(r, c) = rawmean[c].toArray()[r].toInt();
            expmaxraw.at<uint8_t>(r, c) = rawmax[c].toArray()[r].toInt();
            expminraw.at<uint8_t>(r, c) = rawmin[c].toArray()[r].toInt();
            expmaxdff.at<uint8_t>(r, c) = dffmax[c].toArray()[r].toInt();
            expmindff.at<uint8_t>(r, c) = dffmin[c].toArray()[r].toInt();
        }
    }

}

void tVideoData::tload() {
    loadmultipage();
    loadjsonexpected();
    QCOMPARE(data->getNFrames(), 7);
    QCOMPARE(data->getdsSpace(), 1);
    QCOMPARE(data->getdsTime(), 1);
    QCOMPARE(data->getHeight(), 5);
    QCOMPARE(data->getWidth(), 6);
    
    for(size_t i = 0; i<data->getNFrames(); ++i) 
    {
        double minval, maxval;
        cv::minMaxLoc(data->get(false, 0, i) - expraw[i], &minval, &maxval);
        QCOMPARE(maxval, 0);

        cv::minMaxLoc(data->get(true, 0, i) - expdffi[i], &minval, &maxval);
        QVERIFY(maxval <= 1); // allow off-by-one errors, due to rounding (?)
    }
}
void tVideoData::tproj() {
    loadmultipage();
    loadjsonexpected();

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