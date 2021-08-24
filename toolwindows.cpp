#include "toolwindows.h"
#include <QFormLayout>
#include <QBoxLayout>
#include <QStyleOption>
#include <QDebug>
#include <QFileDialog>
#include <qcompleter.h>
#include <qfilesystemmodel.h>
#include <qcombobox.h>
#include <QRadioButton>


using namespace tool;
namespace {
    void addVSep(QVBoxLayout *lay) {
        QFrame* line = new QFrame;
        line->setFrameStyle(QFrame::HLine);
        lay->addWidget(line);
    }
}

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
        const QSize textSize = cmdBrowseFilePath->fontMetrics().size(Qt::TextShowMnemonic, cmdBrowseFilePath->text());
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
        const QSize textSize = cmdBrowseFilePath->fontMetrics().size(Qt::TextShowMnemonic, "999");
        spinFrameRate->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        spinFrameRate->setToolTip(tr("Set the frame rate in which the data were recorded\nin frames per second (FPS)"));
    }

    // DownSampling:
    {
        layMain->addRow(tr("Frame Subset:"), spinDownTime);
        layMain->addRow(tr("Pixel Subset:"), spinDownSpace);

        spinDownTime->setMinimum(1);
        spinDownSpace->setMinimum(1);
        spinDownTime->setMaximum(100);
        spinDownSpace->setMaximum(100);
        spinDownSpace->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        spinDownTime->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
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
    {
        progBar->setMaximum(100);
        progBar->setVisible(false);
        topLay->addWidget(progBar);
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
    connect(spinFrameRate, SIGNAL(valueChanged(double)), this, SIGNAL(frameRateChanged(double)));
}
void imgData::browse()
{
    // check if current path in textbox is valid::
    QString initpath = QDir::currentPath();
    QString currpath_s = txtFilePath->text();
    if (!currpath_s.isEmpty()) {
        QDir currpath_d = QDir(currpath_s);
        QFileInfo currpath_f = QFileInfo(currpath_d.path());
        if (currpath_f.isDir()) { initpath = currpath_s; }
    }
    
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select a Tiff File in the Dataset"), 
                                                    initpath, 
                                                    tr("Tiff Files (*.tif *.tiff)"));

    if (!fileName.isEmpty()) { txtFilePath->setText(QFileInfo(fileName).absolutePath()); }
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

    if (fl.size() > 0) { cmdLoad->setEnabled(true); }
}
void imgData::load()
{
    QFileInfoList fileinfolist = QDir(txtFilePath->text()).entryInfoList(QStringList() << "*.tif" << "*.tiff", QDir::Files);
    QStringList filelist;
    for each (QFileInfo filename in fileinfolist)
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
        lbl += (tr("Loaded ") + QString::number(nframes) + tr(" frames at (") + QString::number(width) + "x" + QString::number(height) + ")");
    }
    else
    {
        lbl += (tr("Loading failed"));
    }
    lblFileInfo->setText(lbl);
}
void imgData::setProgBar(int val) {
    if (val >= 0 && val <= progBar->maximum()) {
        progBar->setVisible(true);
        progBar->setValue(val);
        return;
    }
    progBar->setVisible(false);
}

imgSettings::imgSettings(QWidget* parent) {

    QVBoxLayout* topLay = new QVBoxLayout(this);
    topLay->setAlignment(Qt::AlignTop);

    { // Contrast
        topLay->addWidget(new QLabel(tr("Contrast:")));
        contrast = new ContrastWidget;
        topLay->addWidget(contrast);
        contrast->setMaximumHeight(300);
        contrast->setMaximumWidth(300);
    }
    
    addVSep(topLay);

    { // Projection:
        topLay->addWidget(new QLabel(tr("Projection:")));
        projection = new ProjectionPickWidget;
        projection->setMaximumWidth(280);
        topLay->addWidget(projection);
    }

    addVSep(topLay);
    {// Colormap
        topLay->addWidget(new QLabel(tr("Colormap:")));
        colormap = new ColormapPickWidget;
        colormap->setMaximumWidth(300);
        topLay->addWidget(colormap);
    }
    addVSep(topLay);
    {// Smoothing
        topLay->addWidget(new QLabel(tr("Smoothing:")));
        Wsmoothing = new SmoothingPickWidget;
        Wsmoothing->setMaximumWidth(300);
        topLay->addWidget(Wsmoothing);
        topLay->addStretch(1);
    }

    connect(contrast, &ContrastWidget::contrastChanged, this, &imgSettings::updateSettings);
    connect(projection, &ProjectionPickWidget::projectionChanged, this, &imgSettings::updateSettings);
    connect(colormap, &ColormapPickWidget::colormapChanged, this, &imgSettings::updateSettings);
    connect(Wsmoothing, &SmoothingPickWidget::smoothingChanged, this, &imgSettings::updateSettings);

    setEnabled(false);
}
imgSettings::~imgSettings(){}
void imgSettings::setHistogram(std::vector<float> &data){
    // shim: convert to qvector...(should this come in as qvector? should contrast use std vector?)
    contrast->setHistogram(QVector<float>::fromStdVector(data));
}
void imgSettings::setContrast(ROIVert::contrast c){
    contrast->setContrast(c);
}
void imgSettings::updateSettings() {
    // this is a centralized location for updating settings and emitting a signal with the whole payload:
    ROIVert::imgsettings pay;
    
    pay.Contrast = contrast->getContrast();
    pay.projectionType = projection->getProjection();
    pay.cmap = colormap->getColormap();
    pay.Smoothing = Wsmoothing->getSmoothing();
    
    emit imgSettingsChanged(pay);
}

fileIO::fileIO(QWidget* parent) {
    setParent(parent);
    QVBoxLayout* lay = new QVBoxLayout;
    this->setLayout(lay);

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
            if (!cachepath.isEmpty()) { initpath = cachepath; }
            QString filename = QFileDialog::getSaveFileName(this, tr("Save Traces As..."), initpath, tr("Comma Separated Volume (*.csv)"));
            if (!filename.isEmpty()) {
                cachepath = QFileInfo(filename).absolutePath();
                
                emit exportTraces(filename, chkHeader->isChecked(), chkTime->isChecked());
            }
        }
    );

    connect(cmdExpROIs, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!cachepath.isEmpty()) { initpath = cachepath; }
        QString filename = QFileDialog::getSaveFileName(this, tr("Save ROIs As..."), initpath, tr("JavaScript Object Notation (*.json)"));
        if (!filename.isEmpty()) {
            cachepath = QFileInfo(filename).absolutePath();
            emit exportROIs(filename);
        }
    }
    );

    connect(cmdImpROIs, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!cachepath.isEmpty()) { initpath = cachepath; }
        QString filename = QFileDialog::getOpenFileName(this, tr("Select ROI File..."), initpath, tr("JavaScript Object Notation (*.json)"));
        if (!filename.isEmpty()) {
            cachepath = QFileInfo(filename).absolutePath();
            emit importROIs(filename);
        }
    }
    );

    connect(cmdExpCharts, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!cachepath.isEmpty()) { initpath = cachepath; }
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Chart As (suffix will be added)..."), initpath, tr("Portable Network Graphic (*.png);;Joint Photographic Experts Group file (*.jpeg)"));
        if (!filename.isEmpty()) {
            cachepath = QFileInfo(filename).absolutePath();
            emit exportCharts(filename, spinChartWidth->value(), spinChartHeight->value(), spinChartQuality->value(), false);
        }
    }
    );

    connect(cmdExpRidge, &QPushButton::clicked, this, [=]()
    {
        QString initpath = QDir::currentPath();
        if (!cachepath.isEmpty()) { initpath = cachepath; }
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Chart As..."), initpath, tr("Portable Network Graphic (*.png);;Joint Photographic Experts Group File (*.jpeg)"));
        if (!filename.isEmpty()) {
            cachepath = QFileInfo(filename).absolutePath();
            emit exportCharts(filename, spinChartWidth->value(), spinChartHeight->value(), spinChartQuality->value(), true);
        }
    }
    );
}
