#include "TraceView.h"
#include <QBoxLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <QApplication>
#include <QKeyEvent>

#include "widgets/TraceChartWidget.h"
#include "widgets/RidgeLineWidget.h"
#include "ChartStyle.h"

struct TraceView::pimpl {
    std::unique_ptr<QVBoxLayout> lineChartLayout = std::make_unique<QVBoxLayout>();
    std::unique_ptr<RidgeLineWidget> ridgeChart = std::make_unique<RidgeLineWidget>();
    std::unique_ptr<QGridLayout> topGridLayout = std::make_unique<QGridLayout>();
    
    std::shared_ptr<ChartStyle> coreStyle = std::make_shared<ChartStyle>();

    void doLayout() {
        topGridLayout->addWidget(tab);
        
        tab->addTab(tabLine, "Line");
        tab->addTab(tabRidgeLine, "Ridge");
        tab->addTab(tabImage, "Image");
        
        tabLine->setLayout(scrollAreaParent);
        scrollAreaParent->addWidget(scrollArea);
        scrollAreaContent->setLayout(lineChartLayout.get());
        scrollArea->setWidget(scrollAreaContent);

        scrollArea->setWidgetResizable(true);
        scrollAreaParent->setContentsMargins(0, 0, 0, 0);
        lineChartLayout->setContentsMargins(0, 0, 0, 0);
        lineChartLayout->setSpacing(10);
        lineChartLayout->setMargin(0);

        tabRidgeLine->setLayout(ridgeLayout);
        ridgeLayout->addWidget(ridgeChart.get());
    }

    void scrollToWidget(QWidget* w) {
        scrollArea->ensureWidgetVisible(w);
    }
private:
    // todo: I'd like these all to be unique_ptrs...but I end up with a crash on close presumably because of a double delete?
    // note that Qt will delete all of these as long as everything is parented...which it *should* be
    // (for whatever reason the three public uniqueptrs are fine(?)

    QTabWidget* tab{ new QTabWidget };
    QWidget* tabLine{ new QWidget };
    QWidget* tabRidgeLine{ new QWidget };
    QWidget* tabImage{ new QWidget };

    QGridLayout* scrollAreaParent{ new QGridLayout };
    QWidget* scrollAreaContent{ new QWidget };
    QScrollArea* scrollArea{ new QScrollArea };

    QGridLayout* ridgeLayout{ new QGridLayout };

};


TraceView::TraceView(QWidget* parent) : QWidget(parent) {
    
    impl->ridgeChart->setStyle(impl->coreStyle);

    setLayout(impl->topGridLayout.get());
    impl->doLayout();

    
}

TraceView::~TraceView() {};
    
void TraceView::setTimeLimits(float min, float max) {

}
void TraceView::addLineChart(TraceChartWidget* chart) {
    impl->lineChartLayout->addWidget(chart);
    chart->getXAxis()->setLabel("Time (s)");

    // This extremely aggressive double update is required to ensure that the scroll area
    // is up to date before scrolling to the new chart. 
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    impl->scrollToWidget(chart);
}
void TraceView::scrollToChart(TraceChartWidget* w) {
    impl->scrollToWidget(w);
}
RidgeLineWidget& TraceView::getRidgeChart() noexcept {
    return *(impl->ridgeChart);
}

ChartStyle& TraceView::getCoreChartStyle() {
    return *(impl->coreStyle);
}
void TraceView::keyPressEvent(QKeyEvent* event) {
    emit keyPressed(event->key(), event->modifiers());
}

void TraceView::mousePressEvent(QMouseEvent* event) {
    emit chartClicked(nullptr, std::vector<TraceChartSeries*>(), event->modifiers());
}

