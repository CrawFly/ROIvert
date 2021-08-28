#pragma once
#include <QString>
#include <QVector>
#include <QColor>
#include <QRect>
#include "opencv2/core/types.hpp"


namespace ROIVert {
    /**
     * @brief min, max, gamma
    */
    typedef std::tuple<float, float, float> contrast;
    /**
     * @brief Type, Window, Sigma, SigmaI
    */
    typedef std::tuple<int, int, double, double> smoothing;

    struct imgsettings {
        contrast Contrast{ 0., 1., 1. };
        int projectionType = 0;
        int cmap = -1;
        smoothing Smoothing{ 0, 5, 0., 0. };
    };

    static QString dffstring() {
        //return(QString::fromWCharArray(L"\x03B4\xD835\xDC53/\xD835\xDC53"));
        return "df/f";
    }
}
