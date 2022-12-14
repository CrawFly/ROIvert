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

    QLabel lblTraces{ tr("Traces") };
    QLabel lblCharts{ tr("Charts") };
    QLabel lblROIs{ tr("ROIs") };

    QCheckBox chkHeader{ tr("Include Header") };
    QCheckBox chkTime{ tr("Include Time Column") };
    QPushButton cmdExpTraces{ tr("Export Traces") };

    QSpinBox spinChartWidth;
    QSpinBox spinChartHeight;
    QSpinBox spinChartQuality;

    QPushButton cmdExpCharts{ tr("Export Lines") };
    QPushButton cmdExpRidge{ tr("Export Ridge") };

    QPushButton cmdImpROIs{ tr("Import ROIs") };
    QPushButton cmdExpROIs{ tr("Export ROIs") };

    void init()
    {
        chkHeader.setChecked(true);
        chkTime.setChecked(true);
        cmdExpTraces.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        spinChartWidth.setMinimum(0);
        spinChartWidth.setMaximum(10000);
        spinChartWidth.setValue(1600);
        spinChartWidth.setToolTip(tr("Width of exported images"));

        spinChartHeight.setMinimum(0);
        spinChartHeight.setMaximum(10000);
        spinChartHeight.setValue(600);
        spinChartWidth.setToolTip(tr("Height of exported images"));

        spinChartQuality.setMinimum(10);
        spinChartQuality.setMaximum(100);
        spinChartQuality.setValue(100);
        spinChartQuality.setToolTip(tr("Quality used for compression of images"));
        spinChartQuality.setObjectName("spinChartQuality");

        spinChartWidth.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinChartHeight.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinChartQuality.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

        cmdImpROIs.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        cmdExpROIs.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        cmdExpTraces.setObjectName("cmdExpTraces");
        cmdExpROIs.setObjectName("cmdExpROIs");
        cmdImpROIs.setObjectName("cmdImpROIs");
        cmdExpCharts.setObjectName("cmdExpCharts");
        cmdExpRidge.setObjectName("cmdExpRidge");

        lay.setContentsMargins(10, 0, 10, 10);
    }
    void doLayout()
    {
        contents.setLayout(&lay);
        lay.addWidget(&lblTraces);
        lay.addWidget(&chkHeader);
        lay.addWidget(&chkTime);
        lay.addWidget(&cmdExpTraces);

        {
            QFrame* line = new QFrame;
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
            QFrame* line = new QFrame;
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

FileIOWidget::FileIOWidget(QWidget* parent) : DockWidgetWithSettings(parent), impl(std::make_unique<pimpl>())
{
    toplay.addWidget(&impl->contents);

    impl->init();
    impl->doLayout();

    connect(&impl->cmdExpTraces, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!windowFilePath().isEmpty())
        {
            initpath = windowFilePath();
        }
        QString filename = testoverride ? QDir::currentPath() + "/foo.csv" :
            QFileDialog::getSaveFileName(this, tr("Save Traces As..."), initpath, tr("Comma Separated Volume (*.csv)"));
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
        QString filename = testoverride ? QDir::currentPath() + "/foo.json" :
            QFileDialog::getSaveFileName(this, tr("Save ROIs As..."), initpath, tr("JavaScript Object Notation (*.json)"));
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
        QString filename = testoverride ? QDir::currentPath() + "/foo.json" :
            QFileDialog::getOpenFileName(this, tr("Select ROI File..."), initpath, tr("JavaScript Object Notation (*.json)"));
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
        QString filename = testoverride ? QDir::currentPath() + "/foo.jpeg" :
            QFileDialog::getSaveFileName(this, tr("Save Chart As (suffix will be added)..."), initpath, tr("Portable Network Graphic (*.png);;Joint Photographic Experts Group file (*.jpeg)"));
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
        QString filename = testoverride ? QDir::currentPath() + "/foo.jpeg" :
            QFileDialog::getSaveFileName(this, tr("Save Chart As..."), initpath, tr("Portable Network Graphic (*.png);;Joint Photographic Experts Group File (*.jpeg)"));
        if (!filename.isEmpty())
        {
            setWindowFilePath(QFileInfo(filename).absolutePath());
            emit exportCharts(filename, impl->spinChartWidth.value(), impl->spinChartHeight.value(), impl->spinChartQuality.value(), true);
        }
    });
}
FileIOWidget::~FileIOWidget() { }

void FileIOWidget::setContentsEnabled(bool onoff)
{
    impl->contents.setEnabled(onoff);
}

void FileIOWidget::saveSettings(QSettings& settings) const {
    settings.beginGroup("FileIO");
    settings.setValue("dorestore", getSettingsStorage());
    if (getSettingsStorage()) {
        settings.setValue("header", impl->chkHeader.isChecked());
        settings.setValue("time", impl->chkTime.isChecked());
        settings.setValue("chartwidth", impl->spinChartWidth.value());
        settings.setValue("chartheight", impl->spinChartHeight.value());
        settings.setValue("chartquality", impl->spinChartQuality.value());
    }
    settings.endGroup();
}
void FileIOWidget::restoreSettings(QSettings& settings) {
    settings.beginGroup("FileIO");
    setSettingsStorage(settings.value("dorestore", true).toBool());
    if (getSettingsStorage()) {
        impl->chkHeader.setChecked(settings.value("header", true).toBool());
        impl->chkTime.setChecked(settings.value("time", true).toBool());
        impl->spinChartHeight.setValue(settings.value("chartwidth", 600).toInt());
        impl->spinChartWidth.setValue(settings.value("chartheight", 1600).toInt());
        impl->spinChartQuality.setValue(settings.value("chartquality", 100).toInt());
    }
    settings.endGroup();
}
void FileIOWidget::resetSettings() {
    impl->chkHeader.setChecked(true);
    impl->chkTime.setChecked(true);
    impl->spinChartHeight.setValue(600);
    impl->spinChartWidth.setValue(1600);
    impl->spinChartQuality.setValue(100);
}