#pragma once

// misc shared data types?

struct imgsettings {
    double contrastMin=0.;
    double contrastMax=1.;
    double contrastGamma=1.;
    
    int projectionType=0;
    int cmap=-1;

    int smoothType=0;
    int smoothSize=5;
    double smoothSigma=0.;
    double smoothSimgaI=0.;
};