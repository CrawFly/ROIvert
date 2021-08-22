#pragma once
namespace ROIVert {
    enum class SHAPE {
        RECTANGLE,
        ELLIPSE,
        POLYGON,
        SELECT
    };
    
    enum class NORMALIZATION {
        NONE,
        ZEROTOONE,
        L1NORM,
        L2NORM,
        ZSCORE,
        MEDIQR
    };

    enum class LIMITSTYLE {
        AUTO,
        TIGHT,
        MANAGED
    };
}
