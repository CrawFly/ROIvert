#pragma once

#include <QObject>
#include "imageroiviewer.h"
#include "videodata.h"

class TraceComputer : public QObject
{
    Q_OBJECT

public:
    void update(ImageROIViewer* imView, VideoData* imData, const int ind);   // will update trace at index
    void updateall(ImageROIViewer* imView, VideoData* imData);               // will update all traces (more efficient that iterating over update(ind))
    std::vector<double> getTrace(size_t ind);

signals:
    void traceComputed(const int roiid, const std::vector<double> trace);

private:
    std::vector<std::vector<double>> traces;
    //Note that ind must be <= num traces (can push one), and that it is more efficient to computeTraces than iterate over calls to computeTrace
};
