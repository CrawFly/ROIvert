#pragma once
#include <QString>
#include <QVector>
#include <QColor>
#include <QRect>
#include "opencv2/opencv.hpp"

typedef std::tuple<float, float, float> contrast;

// misc shared data types?
namespace ROIVert {
    struct imgsettings {
        contrast Contrast = std::make_tuple(0., 1., 1.);
        int projectionType = 0;
        int cmap = -1;
        std::tuple<int, int, double, double> smoothing{ 0, 5, 0., 0. };
    };

    static QString dffstring() {
        return(QString::fromWCharArray(L"\x03B4\xD835\xDC53/\xD835\xDC53"));
    }
    static QVector<QColor> colors() {
        QVector<QColor>ret = {
        QColor("#2264A5"),
        QColor("#F75C03"),
        QColor("#F1C40F"),
        QColor("#D90368"),
        QColor("#00CC66")
        };
        return ret;
    }

    static cv::Rect QRect2CVRect(const QRect &bb) {

        return cv::Rect(static_cast<size_t>(bb.x()),
            static_cast<size_t>(bb.y()),
            static_cast<size_t>(bb.width()),
            static_cast<size_t>(bb.height()));
    }


}
