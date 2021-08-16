#pragma once
#include <QObject>
#include <QRect>
class VideoData;
class TraceView;
class ChartStyle;

namespace ROIVert {
    enum class SHAPE;
}
// ROITrace is just a little class that's going to do:
//      Hold a TraceChartWidget
//      Hold a reference to the RidgeLine
//      
//      Do the compute of a trace when roi is edited

class ROITrace : public QObject
{
    Q_OBJECT

public:
    ROITrace(TraceView*, VideoData*, const ChartStyle&);
    ~ROITrace();
    void update();

public slots:
    void updateTrace(ROIVert::SHAPE, QRect, std::vector<QPoint>);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

