#pragma once
#include <QWidget>
#include <memory>

namespace cv {
    class Mat;
}


class TraceView : public QWidget
{
    Q_OBJECT

public:
    TraceView(cv::Mat* DataSource, QWidget* parent = nullptr);
    ~TraceView();
    
    void updateTraces(size_t traceid, bool down);
    void select(size_t traceid);
    size_t getSelected();


    // these are all thin wrappers un update:
    void add(size_t traceid);
    void remove(size_t traceid);
    void edit(size_t traceid);
    void updateAll();

    // save
    // aesthetics
    // xlimits
    // normalization

signals:
    void traceSelected(); // note this is the one from within

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};