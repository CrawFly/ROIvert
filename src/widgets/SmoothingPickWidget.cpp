#include "SmoothingPickWidget.h"
#include <QBoxLayout>

SmoothingPickWidget::SmoothingPickWidget(QWidget* parent) : QWidget(parent) {

    // We'll do box, gaussian, median, bilateral
    // each one has a size
    // gaussian and bilateral have a sigma
    QVBoxLayout* lay = new QVBoxLayout;
    setLayout(lay);

    // Add the combo box items
    cmbBlur->addItem("None");
    cmbBlur->addItem("Box");
    cmbBlur->addItem("Median");
    cmbBlur->addItem("Gaussian");
    cmbBlur->addItem("Bilateral");

    // Set params for sigma and sigma_i spinners:
    spinBlurSigma->setMinimum(0.);
    spinBlurSigma->setMaximum(100.);
    spinBlurSigma->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinBlurSigma->setMaximumWidth(50);
    spinBlurSigmaI->setMinimum(0.);
    spinBlurSigmaI->setMaximum(100.);
    spinBlurSigmaI->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinBlurSigmaI->setMaximumWidth(50);

    // Set size spinner params
    spinBlurSize->setMinimum(0);
    spinBlurSize->setMaximum(50);
    spinBlurSize->setValue(5);
    spinBlurSize->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinBlurSize->setMaximumWidth(50);

    // ParamsLay holds the parameters for smoothing
    QHBoxLayout* paramsLay = new QHBoxLayout;
    lblSigma->setText(QString::fromWCharArray(L"\x03C3S:"));
    lblSigmaI->setText(QString::fromWCharArray(L"\x03C3I:"));

    paramsLay->addWidget(new QLabel(tr("Size:")), 0, Qt::AlignLeft);
    paramsLay->addWidget(spinBlurSize, 0, Qt::AlignLeft);
    paramsLay->addWidget(lblSigma, 0, Qt::AlignLeft);
    paramsLay->addWidget(spinBlurSigma, 0, Qt::AlignLeft);
    paramsLay->addWidget(lblSigmaI, 0, Qt::AlignLeft);
    paramsLay->addWidget(spinBlurSigmaI, 0, Qt::AlignLeft);
    paramsLay->addStretch(1);
    //paramsLay->addWidget(new QWidget, 1);

    widgParams->setLayout(paramsLay);

    lay->addWidget(cmbBlur);
    lay->addWidget(widgParams);
    lay->setSpacing(0);
    widgParams->setVisible(false);

    // set visible params based on smoothing type
    connect(cmbBlur, QOverload<int>::of(&QComboBox::activated),
        [=](int type) {
        widgParams->setVisible(type > 0);
        lblSigma->setVisible(type > 2);
        spinBlurSigma->setVisible(type > 2);
        lblSigmaI->setVisible(type > 3);
        spinBlurSigmaI->setVisible(type > 3);
        }
        );

    // fire signal when anything changes
    //connect(cmbBlur,)
    
    connect(cmbBlur, QOverload<int>::of(&QComboBox::activated), this, &SmoothingPickWidget::smoothingChanged);
    connect(spinBlurSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &SmoothingPickWidget::smoothingChanged);
    connect(spinBlurSigma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SmoothingPickWidget::smoothingChanged);
    connect(spinBlurSigmaI, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SmoothingPickWidget::smoothingChanged);
}

ROIVert::smoothing SmoothingPickWidget::getSmoothing() {
    return std::make_tuple(cmbBlur->currentIndex(),
        spinBlurSize->value(),
        spinBlurSigma->value(),
        spinBlurSigmaI->value());
}

void SmoothingPickWidget::setSmoothing(ROIVert::smoothing s) {
    cmbBlur->setCurrentIndex(std::get<0>(s));
    spinBlurSize->setValue(std::get<1>(s));
    spinBlurSigma->setValue(std::get<2>(s));
    spinBlurSigmaI->setValue(std::get<3>(s));
}
