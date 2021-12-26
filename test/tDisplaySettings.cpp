#include <QtTest/QtTest>
#include "tDisplaySettings.h"
#include "DisplaySettings.h"
#include "opencv2/opencv.hpp"

void tDisplaySettings::initTestCase() {
    mat = new cv::Mat;
    *(mat) = cv::Mat::zeros(4, 5, CV_8U);
    mat->at<uchar>(1, 1) = 100;
    mat->at<uchar>(3, 3) = 150;
    mat->at<uchar>(3, 4) = 200;
}
void tDisplaySettings::init() {
    settings = new DisplaySettings;
}
void tDisplaySettings::cleanup() {
    delete settings;
    settings = nullptr;
}

void tDisplaySettings::cleanupTestCase() {
    delete mat;
    mat = nullptr;
}

void tDisplaySettings::tsetgetcontrast() {
    settings->setContrast(false, { 1, 2, 3 });
    settings->setContrast(true, { 4, 5, 6 });

    QCOMPARE(settings->getContrast(false), ROIVert::contrast({ 1, 2, 3 }));
    QCOMPARE(settings->getContrast(true), ROIVert::contrast({ 4, 5, 6 }));
}

void tDisplaySettings::tsetgetprojectionmode() {
    settings->setProjectionMode(2);
    QCOMPARE(settings->getProjectionMode(), 2);
}

void tDisplaySettings::tsetgetusecolormap() {
    settings->setColormap(2);
    QVERIFY(settings->useCmap());
    settings->setColormap(-1);
    QVERIFY(!settings->useCmap());
}

namespace {
    void checkfivevalues(cv::Mat mat, std::vector<uchar> vals) {
        QCOMPARE(mat.at<uchar>(0, 0), vals[0]);
        QCOMPARE(mat.at<uchar>(2, 2), vals[1]);
        QCOMPARE(mat.at<uchar>(1, 1), vals[2]);
        QCOMPARE(mat.at<uchar>(3, 3), vals[3]);
        QCOMPARE(mat.at<uchar>(3, 4), vals[4]);
    }
}

void tDisplaySettings::tcontrast() {
    cv::Mat pre = mat->clone();

    settings->setContrast(false, ROIVert::contrast({ .2, .8, 1.1 }));
    settings->setContrast(true, ROIVert::contrast({ .1, .9, 0.8 }));

    cv::Mat matraw = settings->getImage(*mat, false);
    cv::Mat matdff = settings->getImage(*mat, true);

    // confirm that mat wasn't touched
    QVERIFY(cv::countNonZero(*mat != pre) == 0);
    checkfivevalues(matraw, { 0, 0, 73, 158, 248 });
    checkfivevalues(matdff, { 0, 0, 114, 172, 225 });
}

void tDisplaySettings::tsmoothing() {
    cv::Mat pre = mat->clone();
    {
        settings->setSmoothing({ (int)smoothingtype::BOX, 1, 0, 0 });
        cv::Mat smoothed = settings->getImage(*mat, false);
        checkfivevalues(smoothed, { 0, 0, 100, 150, 200 });
        // confirm that mat wasn't touched
        QVERIFY(cv::countNonZero(*mat != pre) == 0);
    }
    {
        settings->setSmoothing({ (int)smoothingtype::BOX, 3, 0, 0 });
        cv::Mat smoothed = settings->getImage(*mat, false);
        checkfivevalues(smoothed, { 44, 28, 11, 39, 56 });
    }
    {
        settings->setSmoothing({ (int)smoothingtype::GAUSSIAN, 3, 2, 0 });
        cv::Mat smoothed = settings->getImage(*mat, false);
        checkfivevalues(smoothed, { 41, 26, 13, 42, 60 });
    }
    {
        settings->setSmoothing({ (int)smoothingtype::MEDIAN, 3, 0, 0 });
        cv::Mat smoothed = settings->getImage(*mat, false);
        checkfivevalues(smoothed, { 0, 0, 0, 0, 150 });
    }
    {
        settings->setSmoothing({ (int)smoothingtype::BILATERAL, 3, 2, 100 });
        cv::Mat smoothed = settings->getImage(*mat, false);
        checkfivevalues(smoothed, { 0, 0, 32, 116, 155 });
    }
    {
        settings->setSmoothing({ (int)smoothingtype::NONE, 100, 200, 300 });
        cv::Mat smoothed = settings->getImage(*mat, false);
        checkfivevalues(smoothed, { 0, 0, 100, 150, 200 });
    }
}

void tDisplaySettings::tcolormap() {
    {
        settings->setColormap(2);
        cv::Mat cmapped = settings->getImage(*mat, false);
        auto a = cmapped.at<cv::Vec3b>(1, 1);
        QCOMPARE(a[0], 238);
        QCOMPARE(a[1], 255);
        QCOMPARE(a[2], 18);
    }

    {
        settings->setColormap(3);
        cv::Mat cmapped = settings->getImage(*mat, false);
        auto b = cmapped.at<cv::Vec3b>(1, 1);
        QCOMPARE(b[0], 205);
        QCOMPARE(b[1], 100);
        QCOMPARE(b[2], 0);
    }
}

void tDisplaySettings::tcombined() {
    settings->setContrast(false, ROIVert::contrast({ .1, .9, .4 }));
    settings->setSmoothing({ (int)smoothingtype::GAUSSIAN, 3, 2, 0 });
    cv::Mat smoothed = settings->getImage(*mat, false);
    checkfivevalues(smoothed, { 91, 23, 0, 93, 125 });
}