#pragma once
#include <QWidget>
#include "roivertcore.h"

namespace ContrastWidgetImpl {
    class ContrastChart;
}
class QDoubleSpinBox;


class ContrastWidget : public QWidget {
    Q_OBJECT
public:
    explicit ContrastWidget(QWidget* parent = nullptr);

    // These are all passthru to chart
    void setContrast(ROIVert::contrast c);
    ROIVert::contrast getContrast();
    void setHistogram(QVector<float> y);
    void setHistogramColor(QColor c);
    void setLineColor(QColor c);
    void setGammaRange(qreal mingamma, qreal maxgamma);
signals:
    void contrastChanged(ROIVert::contrast minmaxgamma);

private:
    ContrastWidgetImpl::ContrastChart* chart;
    QDoubleSpinBox* spinMin;
    QDoubleSpinBox* spinGamma;
    QDoubleSpinBox* spinMax;
};
