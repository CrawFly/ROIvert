#include "ResetDialog.h"
#include <QPushButton>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>

ResetDialog::ResetDialog(QWidget* parent) : QDialog(parent) {
    setWindowIcon(QIcon(":/icons/GreenCrown.png"));
    setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    setWindowTitle("ROIVert Reset");

    auto lay = new QVBoxLayout();
    this->setLayout(lay);

    lay->addWidget(new QLabel("Select options to reset to default.\nHover over options for more information."));

    auto chkWindow = new QCheckBox("Window Layout", this);
    auto chkROIColor = new QCheckBox("ROI Colors", this);
    auto chkROIStyle = new QCheckBox("ROI Style", this);
    auto chkChartStyle = new QCheckBox("Chart Style", this);
    auto chkImageData = new QCheckBox("Image Data", this);

    chkWindow->setToolTip(tr("This will reset the default window layout. Windows will be docked and set to the default positions and size"));
    chkROIColor->setToolTip(tr("This will select colors for each ROI from the default palette."));
    chkROIStyle->setToolTip(tr("This will reset the ROI style options (e.g. line width)"));
    chkChartStyle->setToolTip(tr("This will reset all chart display settings"));
    chkImageData->setToolTip(tr("This will reset image data settings like file name, frame rate, etc."));

    lay->addWidget(chkWindow);
    lay->addWidget(chkROIColor);
    lay->addWidget(chkROIStyle);
    lay->addWidget(chkChartStyle);
    lay->addWidget(chkImageData);

    lay->addSpacing(20);

    auto hlay = new QHBoxLayout();
    lay->addLayout(hlay);

    auto cancel = new QPushButton("Cancel", this);
    auto ok = new QPushButton("OK", this);
    cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hlay->addStretch();

    connect(ok, &QPushButton::clicked, this, [=] {
        res.setBit(static_cast<int>(ROIVert::RESET::WINDOW), chkWindow->isChecked());
        res.setBit(static_cast<int>(ROIVert::RESET::ROICOLOR), chkROIColor->isChecked());
        res.setBit(static_cast<int>(ROIVert::RESET::ROISTYLE), chkROIStyle->isChecked());
        res.setBit(static_cast<int>(ROIVert::RESET::CHARTSTYLE), chkChartStyle->isChecked());
        res.setBit(static_cast<int>(ROIVert::RESET::IMAGEDATA), chkImageData->isChecked());
        accept();
    });

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    ok->setDefault(true);
    
    hlay->addWidget(cancel);
    hlay->addWidget(ok);
}
QBitArray ResetDialog::getResult() {
    return res;
}