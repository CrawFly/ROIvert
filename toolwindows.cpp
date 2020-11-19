#include "toolwindows.h"
#include <QFormLayout>
#include <QBoxLayout>
#include <QStyleOption>
#include <QDebug>
#include <QFileDialog>
#include <qcompleter.h>
#include <qfilesystemmodel.h>

using namespace tool;

imgData::imgData(QWidget *parent)
{
    QVBoxLayout *topLay = new QVBoxLayout(this);
    QWidget *widMain = new QWidget(this);
    QFormLayout *layMain = new QFormLayout(widMain);

    topLay->addWidget(widMain);
    topLay->setAlignment(Qt::AlignTop);

    // file path:
    {
        QWidget *w_fp = new QWidget(widMain);
        QHBoxLayout *l_fp = new QHBoxLayout(w_fp);
        l_fp->addWidget(txtFilePath);
        l_fp->addWidget(cmdBrowseFilePath);
        l_fp->setAlignment(Qt::AlignLeft);

        // an autocompleter for the textbox? Not sure why this doesn't work
        QCompleter *completer = new QCompleter(this);
        QFileSystemModel *fsmodel = new QFileSystemModel;
        fsmodel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
        fsmodel->setRootPath(QDir::currentPath());
        completer->setModel(fsmodel);
        completer->setCompletionMode(QCompleter::InlineCompletion);
        txtFilePath->setCompleter(completer);

        txtFilePath->setMinimumWidth(100);
        txtFilePath->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Preferred);

        cmdBrowseFilePath->setText("...");
        auto textSize = cmdBrowseFilePath->fontMetrics().size(Qt::TextShowMnemonic, cmdBrowseFilePath->text());
        QStyleOptionButton opt;
        opt.initFrom(cmdBrowseFilePath);
        opt.rect.setSize(textSize);
        cmdBrowseFilePath->setFixedSize(cmdBrowseFilePath->style()->sizeFromContents(QStyle::CT_PushButton, &opt, textSize, cmdBrowseFilePath));
        l_fp->setContentsMargins(0, 0, 0, 0);
        layMain->addRow(tr("&File Path:"), w_fp);

        txtFilePath->setToolTip(tr("Input a path to a folder that contains a dataset (i.e. tiff files)"));
        cmdBrowseFilePath->setToolTip(tr("Open a dialog to select a folder"));
    }

    // Frame Rate
    {
        layMain->addRow(tr("F&rame Rate:"), spinFrameRate);
        spinFrameRate->setValue(30);
        spinFrameRate->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinFrameRate->setMaximum(999);
        spinFrameRate->setMinimum(1);
        auto textSize = cmdBrowseFilePath->fontMetrics().size(Qt::TextShowMnemonic, "999");
        spinFrameRate->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        spinFrameRate->setToolTip(tr("Set the frame rate in which the data were recorded\nin frames per second (FPS)"));
    }

    // DownSampling:
    {
        layMain->addRow(tr("Frame Subset:"), spinDownTime);
        layMain->addRow(tr("Pixel Subset:"), spinDownSpace);

        /*
        QWidget* w_ds = new QWidget(widMain);
        QFormLayout* l_ds = new QFormLayout(w_ds);
        l_ds->setAlignment(Qt::AlignRight);
        l_ds->addRow("Time", spinDownTime);
        l_ds->addRow("Space", spinDownSpace);
        */
        spinDownTime->setMinimum(1);
        spinDownSpace->setMinimum(1);
        spinDownTime->setMaximum(100);
        spinDownSpace->setMaximum(100);
        spinDownSpace->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        spinDownTime->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        /*
        l_ds->setAlignment(Qt::AlignLeft);
        l_ds->setContentsMargins(0, 0, 0, 0);
        layMain->addRow(tr("&Subset:"),w_ds);

        */
        spinDownTime->setToolTip(tr("Import only every nth frame\nFrame rate should reflect the raw value"));
        spinDownSpace->setToolTip(tr("Import only every nth pixel"));
    }
    topLay->addWidget(widMain);

    // Load Button
    {
        cmdLoad->setText("Load Files");
        cmdLoad->setEnabled(false);
        topLay->addWidget(cmdLoad);
    }
    // Info Label
    {
        topLay->addWidget(lblFileInfo);
        lblFileInfo->setText("Input a File Path");
        lblFileInfo->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Preferred);
    }

    // emit something when framerate changes, and when load is complete...
    connect(cmdBrowseFilePath, &QPushButton::clicked, this, &imgData::browse);
    connect(txtFilePath, &QLineEdit::textChanged, this, &imgData::filePathChanged);
    connect(cmdLoad, &QPushButton::clicked, this, &imgData::load);
    connect(spinFrameRate, SIGNAL(valueChanged(int)), this, SIGNAL(frameRateChanged(int)));

    txtFilePath->setText("C:\\tdeitcher\\");
}
imgData::~imgData() {}

void imgData::browse()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select a Tiff File in the Dataset"), QDir::currentPath(), tr("Tiff Files (*.tif *.tiff)"));

    if (!fileName.isEmpty())
    {
        txtFilePath->setText(QFileInfo(fileName).absolutePath());
    }
}
void imgData::filePathChanged(const QString &filepath)
{
    QFileInfo fp(filepath);
    cmdLoad->setEnabled(false);

    if (!fp.isDir())
    {
        // Not a valid path, set the label, disable the button, return
        lblFileInfo->setText(filepath + tr(" is not a valid path."));
        return;
    }

    QStringList fl = QDir(filepath).entryList(QStringList() << "*.tif"
                                                            << "*.tiff",
                                              QDir::Files);
    lblFileInfo->setText(filepath + tr(" contains ") + QString::number(fl.size()) + tr(" files."));

    if (fl.size() > 0)
    {
        cmdLoad->setEnabled(true);
    }
}

void imgData::load()
{
    // load just emits the load signal with all the data...
    QFileInfoList fileinfolist = QDir(txtFilePath->text()).entryInfoList(QStringList() << "*.tif"
                                                                                       << "*.tiff",
                                                                         QDir::Files);
    QStringList filelist;
    foreach (QFileInfo filename, fileinfolist)
    {
        filelist << filename.absoluteFilePath();
    }

    lblFileInfo->setText(txtFilePath->text() + tr(" contains ") + QString::number(filelist.size()) + tr(" files.\nLoading..."));
    emit fileLoadRequested(filelist, spinFrameRate->value(), spinDownTime->value(), spinDownSpace->value());
}

void imgData::fileLoadCompleted(size_t nframes, size_t height, size_t width)
{
    QStringList filelist = QDir(txtFilePath->text()).entryList(QStringList() << "*.tif"
                                                                             << "*.tiff",
                                                               QDir::Files);
    QString lbl = txtFilePath->text() + tr(" contains ") + QString::number(filelist.size()) + tr(" files.\n");

    if (nframes > 0)
    {
        lbl += (tr("Loaded") + QString::number(nframes) + tr(" frames at (") + QString::number(width) + "x" + QString::number(height) + ")");
    }
    else
    {
        lbl += (tr("Loading failed"));
    }
    lblFileInfo->setText(lbl);
}