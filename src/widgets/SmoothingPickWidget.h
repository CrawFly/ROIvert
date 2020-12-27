/**
 * \class  ProjectionPickWidget.h
 *
 * \brief  Simple buttongroup widget for selecting projection (none/mean/min/max)
 *
 * \author neuroph
*/
#pragma once

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>


class SmoothingPickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SmoothingPickWidget(QWidget* parent = nullptr);
    std::tuple<int,int,double,double> getSmoothing();
    void setSmoothing(std::tuple<int,int,double,double> s);

signals:
    void smoothingChanged();

private:
    QComboBox* cmbBlur = new QComboBox;
    QSpinBox* spinBlurSize = new QSpinBox;
    QDoubleSpinBox* spinBlurSigma = new QDoubleSpinBox;
    QDoubleSpinBox* spinBlurSigmaI = new QDoubleSpinBox;
    QLabel* lblSigma = new QLabel;
    QLabel* lblSigmaI = new QLabel;
    QWidget* widgParams = new QWidget;
};

