#pragma once
#include "widgets/TraceChartWidget.h"

class RidgeLineWidget : public TraceChartWidget
{
public:
    RidgeLineWidget(QWidget* parent = nullptr);
    void updateOffsets();
};

