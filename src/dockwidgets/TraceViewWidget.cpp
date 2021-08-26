#include "dockwidgets/TraceViewWidget.h"
#include <QBoxLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <QApplication>
#include <QKeyEvent>
#include "widgets/TraceChartWidget.h"
#include "ChartStyle.h"
#include "ROIVertEnums.h"

struct TraceViewWidget::pimpl {
    std::unique_ptr<QVBoxLayout> lineChartLayout = std::make_unique<QVBoxLayout>();
    std::unique_ptr<RidgeLineWidget> ridgeChart = std::make_unique<RidgeLineWidget>();
    std::unique_ptr<QGridLayout> topGridLayout = std::make_unique<QGridLayout>();
    
    std::shared_ptr<ChartStyle> coreRidgeStyle = std::make_shared<ChartStyle>();
    std::shared_ptr<ChartStyle> coreLineStyle = std::make_shared<ChartStyle>();

    void doLayout() {
        topGridLayout->addWidget(tab);
        
        tab->addTab(tabLine, "Line");
        tab->addTab(tabRidgeLine, "Ridge");
        //tab->addTab(tabImage, "Image");
        
        tabLine->setLayout(scrollAreaParent);
        scrollAreaParent->addWidget(scrollArea);
        scrollAreaContent->setLayout(lineChartLayout.get());
        scrollArea->setWidget(scrollAreaContent);

        scrollArea->setWidgetResizable(true);
        scrollAreaParent->setContentsMargins(0, 0, 0, 0);
        lineChartLayout->setContentsMargins(0, 0, 0, 0);
        lineChartLayout->setSpacing(10);
        lineChartLayout->setMargin(10);

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


TraceViewWidget::TraceViewWidget(QWidget* parent) : QDockWidget(parent) {
    auto contents = new QWidget;
    this->setWidget(contents);
    contents->setLayout(impl->topGridLayout.get());

    impl->coreRidgeStyle->setDoBackBrush(true);
    impl->coreRidgeStyle->setNormalization(ROIVert::NORMALIZATION::ZEROTOONE);
    impl->coreRidgeStyle->setLimitStyle(ROIVert::LIMITSTYLE::TIGHT);
    impl->ridgeChart->setStyle(impl->coreRidgeStyle);

    setLayout(impl->topGridLayout.get());
    impl->doLayout();
}

TraceViewWidget::~TraceViewWidget() = default;
    
void TraceViewWidget::addLineChart(TraceChartWidget* chart) {
    impl->lineChartLayout->addWidget(chart);
    chart->getXAxis()->setLabel("Time (s)");

    // This extremely aggressive double update is required to ensure that the scroll area
    // is up to date before scrolling to the new chart. 
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    impl->scrollToWidget(chart);
}
void TraceViewWidget::scrollToChart(TraceChartWidget* w) {
    impl->scrollToWidget(w);
}
RidgeLineWidget& TraceViewWidget::getRidgeChart() noexcept {
    return *(impl->ridgeChart);
}

ChartStyle* TraceViewWidget::getCoreRidgeChartStyle() const noexcept {
    return impl->coreRidgeStyle.get();
}
ChartStyle* TraceViewWidget::getCoreLineChartStyle() const noexcept {
    return impl->coreLineStyle.get();
}
void TraceViewWidget::keyPressEvent(QKeyEvent* event) {
    emit keyPressed(event->key(), event->modifiers());
}

void TraceViewWidget::mousePressEvent(QMouseEvent* event) {
    emit chartClicked(nullptr, std::vector<TraceChartSeries*>(), event->modifiers());
}
