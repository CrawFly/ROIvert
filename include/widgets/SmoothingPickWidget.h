/**
 * \class  ProjectionPickWidget.h
 *
 * \brief  Simple buttongroup widget for selecting projection (none/mean/min/max)
 *
 * \author neuroph
*/
#pragma once

#include <QWidget>
#include "roivertcore.h"

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QLabel;

class SmoothingPickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SmoothingPickWidget(QWidget* parent = nullptr);
    ROIVert::smoothing getSmoothing();
    void setSmoothing(ROIVert::smoothing s);
    void updateSmothingParamWidgets();


signals:
    void smoothingChanged();

private:
    QComboBox* cmbBlur;
    QSpinBox* spinBlurSize;
    QDoubleSpinBox* spinBlurSigma;
    QDoubleSpinBox* spinBlurSigmaI;
    QLabel* lblSigma;
    QLabel* lblSigmaI;
    QWidget* widgParams;
};

