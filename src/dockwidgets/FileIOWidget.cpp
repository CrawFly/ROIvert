#include "dockwidgets/FileIOWidget.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QSPinBox>
#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QDir>
#include <QFileDialog>

FileIOWidget::FileIOWidget(QWidget* parent)  : QDockWidget(parent) {

    contents = new QWidget;
    this->setWidget(contents);
    QVBoxLayout* lay = new QVBoxLayout;
    contents->setLayout(lay);

    QCheckBox* chkHeader = new QCheckBox("Include Header");
    chkHeader->setChecked(true);
    QCheckBox* chkTime = new QCheckBox("Include Time Column");
    chkTime->setChecked(true);
    QHBoxLayout* optLay = new QHBoxLayout;
    optLay->addStretch(1);
    QPushButton* cmdExpTraces = new QPushButton("Export Traces");
    cmdExpTraces->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    lay->addWidget(new QLabel("Traces"));
    lay->addWidget(chkHeader);
    lay->addWidget(chkTime);
    lay->addLayout(optLay);
    lay->addWidget(cmdExpTraces);

    {
        QFrame* line = new QFrame;
        line->setFrameStyle(QFrame::HLine);
        lay->addWidget(line);
    }


    lay->addWidget(new QLabel("Charts"));
    QSpinBox* spinChartWidth = new QSpinBox;
    QSpinBox* spinChartHeight = new QSpinBox;
    QSpinBox* spinChartQuality = new QSpinBox;

    spinChartWidth->setMinimum(0);
    spinChartWidth->setMaximum(10000);
    spinChartWidth->setValue(1600); 
    
    spinChartHeight->setMinimum(0);
    spinChartHeight->setMaximum(10000);
    spinChartHeight->setValue(600);
    
    spinChartQuality->setMinimum(10);
    spinChartQuality->setMaximum(100);
    spinChartQuality->setValue(100);

    spinChartWidth->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinChartHeight->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinChartQuality->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);


    QGridLayout* layChartParams = new QGridLayout;

    layChartParams->addWidget(new QLabel("Dimensions:"),0,0);
    layChartParams->addWidget(spinChartWidth,0,1);
    layChartParams->addWidget(new QLabel(" x "),0,2);
    layChartParams->addWidget(spinChartHeight,0,3);
    layChartParams->setAlignment(Qt::AlignLeft);
    layChartParams->addWidget(new QLabel("Quality:"),1,0);
    layChartParams->addWidget(spinChartQuality,1,1);

    QHBoxLayout* layChartButs = new QHBoxLayout;
    QPushButton* cmdExpCharts = new QPushButton("Export Lines", this);
    QPushButton* cmdExpRidge = new QPushButton("Export Ridge", this);

    //cmdExpCharts->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    layChartButs->addWidget(cmdExpCharts);
    layChartButs->addWidget(cmdExpRidge);
    layChartButs->addStretch(1);

    lay->addLayout(layChartParams);
    lay->addLayout(layChartButs);
    {
        QFrame* line = new QFrame;
        line->setFrameStyle(QFrame::HLine);
        lay->addWidget(line);
    }

    lay->addWidget(new QLabel("ROIs"));
    QPushButton* cmdImpROIs = new QPushButton("Import ROIs");
    QPushButton* cmdExpROIs = new QPushButton("Export ROIs");
    QHBoxLayout* layroi = new QHBoxLayout;
    
    layroi->addWidget(cmdImpROIs);
    layroi->addWidget(cmdExpROIs);
    layroi->addStretch(1);
    lay->addLayout(layroi);

    cmdImpROIs->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    cmdExpROIs->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    
    lay->addStretch(1);

    connect(cmdExpTraces, &QPushButton::clicked, this, [=]()
        {
            QString initpath = QDir::currentPath();
            if (!windowFilePath().isEmpty()) { initpath = windowFilePath(); }
            QString filename = QFileDialog::getSaveFileName(this, tr("Save Traces As..."), initpath, tr("Comma Separated Volume (*.csv)"));
            if (!filename.isEmpty()) {
                windowFilePath() = QFileInfo(filename).absolutePath();
                
                emit exportTraces(filename, chkHeader->isChecked(), chkTime->isChecked());
            }
        }
    );

    connect(cmdExpROIs, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!windowFilePath().isEmpty()) { initpath = windowFilePath(); }
        QString filename = QFileDialog::getSaveFileName(this, tr("Save ROIs As..."), initpath, tr("JavaScript Object Notation (*.json)"));
        if (!filename.isEmpty()) {
            windowFilePath() = QFileInfo(filename).absolutePath();
            emit exportROIs(filename);
        }
    }
    );

    connect(cmdImpROIs, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!windowFilePath().isEmpty()) { initpath = windowFilePath(); }
        QString filename = QFileDialog::getOpenFileName(this, tr("Select ROI File..."), initpath, tr("JavaScript Object Notation (*.json)"));
        if (!filename.isEmpty()) {
            setWindowFilePath(QFileInfo(filename).absolutePath());
            emit importROIs(filename);
        }
    }
    );

    connect(cmdExpCharts, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!windowFilePath().isEmpty()) { initpath = windowFilePath(); }
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Chart As (suffix will be added)..."), initpath, tr("Portable Network Graphic (*.png);;Joint Photographic Experts Group file (*.jpeg)"));
        if (!filename.isEmpty()) {
            setWindowFilePath(QFileInfo(filename).absolutePath());
            emit exportCharts(filename, spinChartWidth->value(), spinChartHeight->value(), spinChartQuality->value(), false);
        }
    }
    );

    connect(cmdExpRidge, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!windowFilePath().isEmpty()) { initpath = windowFilePath(); }
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Chart As..."), initpath, tr("Portable Network Graphic (*.png);;Joint Photographic Experts Group File (*.jpeg)"));
        if (!filename.isEmpty()) {
            setWindowFilePath(QFileInfo(filename).absolutePath());
            emit exportCharts(filename, spinChartWidth->value(), spinChartHeight->value(), spinChartQuality->value(), true);
        }
    }
    );
}
void FileIOWidget::setContentsEnabled(bool onoff) {
    if (contents) {
        contents->setEnabled(onoff);
    }
}
