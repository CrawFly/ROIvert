#include "widgets/TraceChartWidget.h"

RidgeLineWidget::RidgeLineWidget(std::shared_ptr<ChartStyle> style, QWidget *parent) : TraceChartWidget(style, parent)
{
    // do custom setup of visuals here...
    getYAxis()->setVisible(false);
    getXAxis()->setLabel("Time (s)");
}
void RidgeLineWidget::updateOffsets()
{
    float i = 0;
    for (auto &ser : getSeries())
    {
        ser->setOffset(-offset * i++);
    }
    updateExtents();
    update();
}