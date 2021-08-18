#pragma once
#include "ROI/ROIs.h"
#include "TraceView.h"
#include "VideoData.h"

class FileIO
{
public:
    FileIO(ROIs*, TraceView*, VideoData*);
    ~FileIO();
    void exportTraces(QString filename, bool includeheader, bool includetime) const;
    void importROIs(QString filename) const;
    void exportROIs(QString filename) const;
    void exportLineCharts(int width, int height, int quality) const;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

