#pragma once
#include <QWidget>
#include <memory>

namespace cv {
    class Mat;
}

class TraceChartWidget;

class TraceView : public QWidget
{
    Q_OBJECT

public:
    TraceView(cv::Mat* DataSource, QWidget* parent = nullptr);
    ~TraceView();
    
    void updateTraces(size_t traceid=1, bool down=true);
    void select(size_t traceid);
    size_t getSelected();

    void removeTrace(size_t traceid);
    void connectChartSelect(TraceChartWidget* chart, size_t traceid);
    void setTimeLimits(float min, float max);

    void exportCharts(QString filename, int width, int height, int quality, bool ridge);

    // aesthetics
    // xlimits
    // normalization
protected:
    void keyPressEvent(QKeyEvent* event) override;

signals:
    void traceSelected(size_t roiid); 
    void roiDeleted(size_t roiid);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};