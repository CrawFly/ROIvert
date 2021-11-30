#pragma once
#include <QObject>

class DisplaySettings;
namespace cv {
    class Mat;
}

class tDisplaySettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    
    void tsetgetcontrast();
    void tsetgetprojectionmode();
    void tsetgetusecolormap();

    void tcontrast();
    void tsmoothing();
    void tcolormap();

    void tcombined();


private:
    DisplaySettings* settings = nullptr;
    cv::Mat* mat;
};

