#pragma once
#include <QObject>
#include "opencv2/opencv.hpp"

class VideoData;

class tVideoData : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    
    void tload_data();
    void tload();
    void tproj_raw();
    void tproj_dff();
    
    void tdowns_data();
    void tdowns();
    void tdownt_data();
    void tdownt();

    void tfr();

    /*
    
    void ttrace();
    void tdeadpixel();

    void tmultifile();
    void tmultifile_data();

    void temptyfilelist();
    void tgetoverflow();
    void tnowidthtrace();

    void thistogram();
*/
private:
    VideoData* data;
};

//todo:
//  16 bit datasets
