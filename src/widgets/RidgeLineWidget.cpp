#include "widgets/RidgeLineWidget.h"


RidgeLineWidget::RidgeLineWidget(QWidget* parent) : TraceChartWidget(parent) {
    // do custom setup of visuals here...
}
void RidgeLineWidget::updateOffsets() {
    float i = 0;
    for (auto& ser : getSeries()) {
        ser->setOffset(-.5 * i++);
    }
}
