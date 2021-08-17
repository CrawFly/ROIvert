#pragma once
#include "widgets/TraceChartWidget.h"

class RidgeLineWidget : public TraceChartWidget
{
public:
    RidgeLineWidget(std::shared_ptr<ChartStyle> = nullptr, QWidget* parent = nullptr);
    void updateOffsets();
};

