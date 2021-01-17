#include <TraceView.h>
#include "opencv2/opencv.hpp"
#include "widgets/TraceChartWidget.h"
#include "roivertcore.h"

#include <QBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

//**** note always that traceid is 1 based, it's the roi number not the 

struct TraceView::pimpl {
    // setParent?
    void setData(cv::Mat* datasource) { data = datasource; } // if this is exposed ourside of TraceView constructor, will need to add some clear logic here
    void updateTraces(std::vector<size_t> ids) {
        // assumes sorted ascending
        for (auto& id : ids) {
            // if there's no chart at this id, add one
            if (id > LineCharts.size()) {
                LineCharts.push_back(new TraceChartWidget);
                lay->addWidget(LineCharts[id - 1]);
                LineCharts[id - 1]->setXLabel("Time (s)");
                LineCharts[id - 1]->setTitle("ROI " + QString::number(id));
                LineCharts[id - 1]->setContentsMargins(0, 0, 0, 0);
                LineCharts[id - 1]->setMinimumHeight(350);
                LineCharts[id - 1]->setYLabel(ROIVert::dffstring());
            }
            LineCharts[id - 1]->setData(data->row(id - 1), "trace", 0, 1);
        }

        // remove any charts that aren't in data:
        for (int i = data->size().height; i < LineCharts.size(); ++i) {
            LineCharts.erase(LineCharts.begin() + i);
        }

        // Ridge goes here...
    };

    size_t getDataSize() { return data->size().height; };

    void setSelected(size_t traceid) { selected = traceid; }; //todo: update colors
    size_t getSelected(){ return selected; }

    QVBoxLayout* lay = new QVBoxLayout;
    QScrollArea* scroll = new QScrollArea;

private:
    cv::Mat* data;
    std::vector<TraceChartWidget*> LineCharts; // move to list?
    //TraceChartWidget* RidgeChart;
    size_t selected = 0;

};

TraceView::TraceView(cv::Mat* DataSource, QWidget* parent)
    : QWidget(parent) {

    QGridLayout* glay = new QGridLayout;
    setLayout(glay);
    
    QWidget* scrollAreaContent = new QWidget;
    scrollAreaContent->setLayout(impl->lay);
    impl->scroll->setWidget(scrollAreaContent);
    impl->scroll->setWidgetResizable(true);
    impl->lay->setAlignment(Qt::AlignTop);
    impl->lay->setContentsMargins(0, 0, 0, 0);
    glay->addWidget(impl->scroll);
    
    
    impl->setData(DataSource);
}

TraceView::~TraceView() = default;

void TraceView::updateTraces(size_t traceid, bool down){
    size_t sz = down ? impl->getDataSize() - traceid + 1 : 1;
    std::vector<size_t> tracelist(sz);
    std::iota(tracelist.begin(), tracelist.end(), traceid);
    impl->updateTraces(tracelist);
}

void TraceView::select(size_t traceid){}
size_t TraceView::getSelected() { return impl->getSelected(); }

// these are all thin wrappers un update:
void TraceView::add(size_t traceid) { updateTraces(traceid, true); }
void TraceView::remove(size_t traceid) { updateTraces(traceid, true); }
void TraceView::edit(size_t traceid) { updateTraces(traceid, false); }
void TraceView::updateAll(){ updateTraces(1, true); };
