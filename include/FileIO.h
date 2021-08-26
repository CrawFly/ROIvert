#pragma once
#include "ROI/ROIs.h"
#include "dockwidgets/TraceViewWidget.h"
#include "VideoData.h"

class FileIO
{
public:
    FileIO(ROIs*, TraceViewWidget*, VideoData*);
    ~FileIO();
    void exportTraces(QString filename, bool includeheader, bool includetime) const;
    void importROIs(QString filename) const;
    void exportROIs(QString filename) const;
    void exportCharts(QString filename, int width, int height, int quality, bool ridge);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

