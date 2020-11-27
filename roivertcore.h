#pragma once
#include <QString>

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
}
