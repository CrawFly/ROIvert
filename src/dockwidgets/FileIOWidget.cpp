#include "dockwidgets/FileIOWidget.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

struct FileIOWidget::pimpl
{
    QWidget contents;
    QVBoxLayout lay;
    QFormLayout layChartParams;

    QLabel lblTraces{tr("Traces")};
    QLabel lblCharts{tr("Charts")};
    QLabel lblROIs{tr("ROIs")};

    QCheckBox chkHeader{tr("Include Header")};
    QCheckBox chkTime{tr("Include Time Column")};
    QPushButton cmdExpTraces{tr("Export Traces")};

    QSpinBox spinChartWidth;
    QSpinBox spinChartHeight;
    QSpinBox spinChartQuality;

    QPushButton cmdExpCharts{tr("Export Lines")};
    QPushButton cmdExpRidge{tr("Export Ridge")};

    QPushButton cmdImpROIs{tr("Import ROIs")};
    QPushButton cmdExpROIs{tr("Export ROIs")};

    void init()
    {
        chkHeader.setChecked(true);
        chkTime.setChecked(true);
        cmdExpTraces.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        spinChartWidth.setMinimum(0);
        spinChartWidth.setMaximum(10000);
        spinChartWidth.setValue(1600);

        spinChartHeight.setMinimum(0);
        spinChartHeight.setMaximum(10000);
        spinChartHeight.setValue(600);

        spinChartQuality.setMinimum(10);
        spinChartQuality.setMaximum(100);
        spinChartQuality.setValue(100);

        spinChartWidth.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinChartHeight.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinChartQuality.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

        cmdImpROIs.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        cmdExpROIs.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    }
    void doLayout()
    {
        contents.setLayout(&lay);
        lay.addWidget(&lblTraces);
        lay.addWidget(&chkHeader);
        lay.addWidget(&chkTime);
        lay.addWidget(&cmdExpTraces);

        {
            QFrame *line = new QFrame;
            line->setFrameStyle(QFrame::HLine);
            lay.addWidget(line);
        }

        lay.addWidget(&lblCharts);
        {
            auto templay = std::make_unique<QHBoxLayout>();
            auto templbl = std::make_unique<QLabel>(" x ");
            templay->addWidget(&spinChartWidth);
            templay->addWidget(templbl.release());
            templay->addWidget(&spinChartHeight);
            templay->addStretch();
            layChartParams.addRow(tr("Dimensions"), templay.release());
        }
        {
            auto templay = std::make_unique<QHBoxLayout>();
            templay->addWidget(&spinChartQuality);
            templay->addStretch();
            layChartParams.addRow(tr("Quality"), templay.release());
        }

        lay.addLayout(&layChartParams);
        {
            auto templay = std::make_unique<QHBoxLayout>();
            templay->addWidget(&cmdExpCharts);
            templay->addWidget(&cmdExpRidge);
            templay->addStretch(1);
            lay.addLayout(templay.release());
        }

        {
            QFrame *line = new QFrame;
            line->setFrameStyle(QFrame::HLine);
            lay.addWidget(line);
        }

        lay.addWidget(&lblROIs);
        {
            auto templay = std::make_unique<QHBoxLayout>();
            templay->addWidget(&cmdImpROIs);
            templay->addWidget(&cmdExpROIs);
            templay->addStretch(1);
            lay.addLayout(templay.release());
        }
        
        lay.addStretch();
    }
};

FileIOWidget::FileIOWidget(QWidget *parent) : QDockWidget(parent)
{
    setWidget(&impl->contents);

    impl->init();
    impl->doLayout();

    connect(&impl->cmdExpTraces, &QPushButton::clicked, this, [=]()
            {
                QString initpath = QDir::currentPath();
                if (!windowFilePath().isEmpty())
                {
                    initpath = windowFilePath();
                }
                QString filename = QFileDialog::getSaveFileName(this, tr("Save Traces As..."), initpath, tr("Comma Separated Volume (*.csv)"));
                if (!filename.isEmpty())
                {
                    windowFilePath() = QFileInfo(filename).absolutePath();

                    emit exportTraces(filename, impl->chkHeader.isChecked(), impl->chkTime.isChecked());
                }
            });

    connect(&impl->cmdExpROIs, &QPushButton::clicked, this, [=]()
            {
                QString initpath = QDir::currentPath();
                if (!windowFilePath().isEmpty())
                {
                    initpath = windowFilePath();
                }
                QString filename = QFileDialog::getSaveFileName(this, tr("Save ROIs As..."), initpath, tr("JavaScript Object Notation (*.json)"));
                if (!filename.isEmpty())
                {
                    windowFilePath() = QFileInfo(filename).absolutePath();
                    emit exportROIs(filename);
                }
            });

    connect(&impl->cmdImpROIs, &QPushButton::clicked, this, [=]()
            {
                QString initpath = QDir::currentPath();
                if (!windowFilePath().isEmpty())
                {
                    initpath = windowFilePath();
                }
                QString filename = QFileDialog::getOpenFileName(this, tr("Select ROI File..."), initpath, tr("JavaScript Object Notation (*.json)"));
                if (!filename.isEmpty())
                {
                    setWindowFilePath(QFileInfo(filename).absolutePath());
                    emit importROIs(filename);
                }
            });

    connect(&impl->cmdExpCharts, &QPushButton::clicked, this, [=]()
            {
                QString initpath = QDir::currentPath();
                if (!windowFilePath().isEmpty())
                {
                    initpath = windowFilePath();
                }
                QString filename = QFileDialog::getSaveFileName(this, tr("Save Chart As (suffix will be added)..."), initpath, tr("Portable Network Graphic (*.png);;Joint Photographic Experts Group file (*.jpeg)"));
                if (!filename.isEmpty())
                {
                    setWindowFilePath(QFileInfo(filename).absolutePath());
                    emit exportCharts(filename, impl->spinChartWidth.value(), impl->spinChartHeight.value(), impl->spinChartQuality.value(), false);
                }
            });

    connect(&impl->cmdExpRidge, &QPushButton::clicked, this, [=]()
            {
                QString initpath = QDir::currentPath();
                if (!windowFilePath().isEmpty())
                {
                    initpath = windowFilePath();
                }
                QString filename = QFileDialog::getSaveFileName(this, tr("Save Chart As..."), initpath, tr("Portable Network Graphic (*.png);;Joint Photographic Experts Group File (*.jpeg)"));
                if (!filename.isEmpty())
                {
                    setWindowFilePath(QFileInfo(filename).absolutePath());
                    emit exportCharts(filename, impl->spinChartWidth.value(), impl->spinChartHeight.value(), impl->spinChartQuality.value(), true);
                }
            });
}
void FileIOWidget::setContentsEnabled(bool onoff)
{
    impl->contents.setEnabled(onoff);
}
