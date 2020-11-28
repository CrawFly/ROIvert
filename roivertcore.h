#pragma once
#include <QString>
#include <QVector>
#include <QColor>

// misc shared data types?
namespace ROIVert {
    struct imgsettings {
        double contrastMin = 0.;
        double contrastMax = 1.;
        double contrastGamma = 1.;

        int projectionType = 0;
        int cmap = -1;

        int smoothType = 0;
        int smoothSize = 5;
        double smoothSigma = 0.;
        double smoothSimgaI = 0.;
    };

    static const QString dffstring(QString::fromWCharArray(L"\x03B4\xD835\xDC53/\xD835\xDC53"));

    static const QVector<QColor> colors = {
        QColor("#2264A5"),
        QColor("#F75C03"),
        QColor("#F1C40F"),
        QColor("#D90368"),
        QColor("#00CC66")
    };


    // colors: (?)
    //  2274A5
    //  F75C03
    //  F1C40F
    //  D90368
    //  00CC66
}
