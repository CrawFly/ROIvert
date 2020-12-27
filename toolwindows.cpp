#include "toolwindows.h"
#include <QFormLayout>
#include <QBoxLayout>
#include <QStyleOption>
#include <QDebug>
#include <QFileDialog>
#include <qcompleter.h>
#include <qfilesystemmodel.h>
#include <QtCharts/qvalueaxis.h>
#include <QtCharts/qareaseries.h>
#include <qcombobox.h>
#include <opencv2/opencv.hpp>
#include <QRadioButton>


using namespace tool;
using namespace QtCharts;


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
    // load just emits the load signal with all the data...
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
namespace {
    std::vector<QPixmap> getColormapPixmaps(const cv::ColormapTypes maps[5]) {
        unsigned char data[20][128];

        // todo: consider replacing with std::iota and vector?
        for (int i = 0; i < 128; i++) {
            for (int j = 0; j < 20; j++) {
                data[j][i] = i*2;
            }
        }
        cv::Mat cv_cmap(10, 128, CV_8U, data);


        // first map is always b/w
        std::vector<QPixmap> res;
        res.push_back(QPixmap::fromImage(QImage(cv_cmap.data, cv_cmap.size().width, cv_cmap.size().height, cv_cmap.step, QImage::Format_Grayscale8)));

        for (int i = 0; i < 5; i++) {
            cv::applyColorMap(cv_cmap, cv_cmap, maps[i]);
            res.push_back(QPixmap::fromImage(QImage(cv_cmap.data, cv_cmap.size().width, cv_cmap.size().height, cv_cmap.step, QImage::Format_BGR888)));
        }
        return res;
    }
}
imgSettings::imgSettings(QWidget* parent) {

    QVBoxLayout* topLay = new QVBoxLayout(this);
    topLay->setAlignment(Qt::AlignTop);

    { // Contrast
        topLay->addWidget(new QLabel(tr("Contrast:")));
        contrast = new ContrastPickWidget;
        topLay->addWidget(contrast);
        contrast->setMaximumHeight(300);
        contrast->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }
    {
        QFrame* line = new QFrame;
        line->setFrameStyle(QFrame::HLine);
        topLay->addWidget(line);
    }
    { // Projection:
        topLay->addWidget(new QLabel(tr("Projection:")));
        projection = new ProjectionPickWidget;
        topLay->addWidget(projection);
    }


    {
        QFrame* line = new QFrame;
        line->setFrameStyle(QFrame::HLine);
        topLay->addWidget(line);
    }

    {// Colormap
        QFormLayout* flay = new QFormLayout;
        cmbColormap = new QComboBox;

        std::vector<QPixmap> c = getColormapPixmaps(cmaps);
        for (int i = 0; i < c.size(); i++) {
            cmbColormap->addItem("");
            cmbColormap->setItemData(i, c[i], Qt::DecorationRole);
        }
        topLay->addWidget(cmbColormap);

        QSize sz;
        sz.setWidth(127);
        sz.setHeight(20);
        cmbColormap->setIconSize(sz);

        cmbColormap->setMaximumWidth(175);
        flay->addRow(tr("Colormap:"), cmbColormap);
        topLay->addLayout(flay);
    }
    {
        QFrame* line = new QFrame;
        line->setFrameStyle(QFrame::HLine);
        topLay->addWidget(line);
    }
    {// Smoothing
        // We'll do box, gaussian, median, bilateral
        // each one has a size
        // gaussian and bilateral have a sigma
        topLay->addWidget(new QLabel(tr("Smoothing:")));
        QVBoxLayout* blurLay = new QVBoxLayout;
        cmbBlur = new QComboBox;
        cmbBlur->addItem("None");
        cmbBlur->addItem("Box");
        cmbBlur->addItem("Median");
        cmbBlur->addItem("Gaussian");
        cmbBlur->addItem("Bilateral");
        
        spinBlurSigma = new QDoubleSpinBox;
        spinBlurSigma->setMinimum(0.);
        spinBlurSigma->setMaximum(100.);
        spinBlurSigma->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

        spinBlurSigmaI = new QDoubleSpinBox;
        spinBlurSigmaI->setMinimum(0.);
        spinBlurSigmaI->setMaximum(100.);
        spinBlurSigmaI->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

        spinBlurSize = new QSpinBox;
        spinBlurSize->setMinimum(0);
        spinBlurSize->setMaximum(50);
        spinBlurSize->setValue(5);
        spinBlurSize->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

        spinBlurSigma->setMaximumWidth(50);
        spinBlurSigmaI->setMaximumWidth(50);
        spinBlurSize->setMaximumWidth(50);

        QHBoxLayout* paramsLay = new QHBoxLayout;
        lblSigma = new QLabel(QString::fromWCharArray(L"\x03C3S:"));
        lblSigmaI = new QLabel(QString::fromWCharArray(L"\x03C3I:"));
        
        paramsLay->addWidget(new QLabel(tr("Size:")), 0, Qt::AlignLeft);
        paramsLay->addWidget(spinBlurSize, 0, Qt::AlignLeft);
        paramsLay->addWidget(lblSigma, 0, Qt::AlignLeft);
        paramsLay->addWidget(spinBlurSigma,0,Qt::AlignLeft);
        paramsLay->addWidget(lblSigmaI, 0, Qt::AlignLeft);
        paramsLay->addWidget(spinBlurSigmaI, 0, Qt::AlignLeft);
        paramsLay->addWidget(new QWidget, 1);

        paramsWidg = new QWidget;
        paramsWidg->setLayout(paramsLay);


        blurLay->addWidget(cmbBlur);
        blurLay->addWidget(paramsWidg);
        blurLay->setSpacing(0);
        paramsWidg->setVisible(false);
        topLay->addLayout(blurLay,1);

        connect(cmbBlur, QOverload<int>::of(&QComboBox::activated),
            [=](int type) {
            paramsWidg->setVisible(type > 0);
            lblSigma->setVisible(type > 2);
            spinBlurSigma->setVisible(type > 2);
            lblSigmaI->setVisible(type > 3);
            spinBlurSigmaI->setVisible(type > 3);
        }
        );
        topLay->addStretch(1);
    }

    connect(cmbColormap, QOverload<int>::of(&QComboBox::activated), this, &imgSettings::updateSettings);
    connect(cmbBlur, QOverload<int>::of(&QComboBox::activated), this, &imgSettings::updateSettings);
    connect(spinBlurSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &imgSettings::updateSettings);
    connect(spinBlurSigma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &imgSettings::updateSettings);
    connect(spinBlurSigmaI, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &imgSettings::updateSettings);
    connect(contrast, &ContrastPickWidget::contrastChanged, this, &imgSettings::updateSettings);
    connect(projection, &ProjectionPickWidget::projectionChanged, this, &imgSettings::updateSettings);
    setEnabled(false);
}
imgSettings::~imgSettings(){}
void imgSettings::setHistogram(std::vector<float> &data){
    contrast->setHist(data);
}
void imgSettings::setContrast(float min, float max, float gamma){
    contrast->setContrast(min, max, gamma);
}
void imgSettings::updateSettings() {
    // this is a centralized location for updating settings and emitting a signal with the whole payload:
    ROIVert::imgsettings pay;
    
    pay.contrastMin = contrast->getMin();
    pay.contrastMax = contrast->getMax();
    pay.contrastGamma = contrast->getGamma();
    pay.projectionType = projection->getProjection();
    
    const int cmapIndex = cmbColormap->currentIndex();
    if (cmapIndex == 0) { pay.cmap = -1;}
    else { pay.cmap = cmaps[cmapIndex - 1]; }

    pay.smoothType = cmbBlur->currentIndex();
    pay.smoothSize = spinBlurSize->value();
    pay.smoothSigma = spinBlurSigma->value();
    pay.smoothSimgaI = spinBlurSigmaI->value();

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
    spinChartWidth->setMinimum(0);
    spinChartHeight->setMinimum(0);
    spinChartWidth->setMaximum(10000);
    spinChartHeight->setMaximum(10000);

    spinChartWidth->setValue(1600);
    spinChartHeight->setValue(600);
    spinChartWidth->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    spinChartHeight->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

    QHBoxLayout* layChartDim = new QHBoxLayout;
    layChartDim->addWidget(new QLabel("Dimensions:"));
    layChartDim->addWidget(spinChartWidth);
    layChartDim->addWidget(new QLabel(" x "));
    layChartDim->addWidget(spinChartHeight);
    layChartDim->addStretch(1);
    QCheckBox* chkChartTitle = new QCheckBox("Include Title");

    QPushButton* cmdExpCharts = new QPushButton("Export Charts", this);
    cmdExpCharts->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    lay->addLayout(layChartDim);
    lay->addWidget(chkChartTitle);
    lay->addWidget(cmdExpCharts);

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
        QString filename = QFileDialog::getSaveFileName(this, tr("Save ROIs As..."), initpath, tr("Extensible Markup Language (*.xml)"));
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
        //QString filename = QFileDialog::getSaveFileName(this, tr("Save ROIs As..."), initpath, tr("Extensible Markup Language (*.xml)"));
        QString filename = QFileDialog::getOpenFileName(this, tr("Select ROI File..."), initpath, tr("Extensible Markup Language (*.xml)"));
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
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Chart As (suffix will be added)..."), initpath, tr("Portable Network Graphic (*.png)"));
        if (!filename.isEmpty()) {
            cachepath = QFileInfo(filename).absolutePath();
            emit exportCharts(filename, chkChartTitle->isChecked(), spinChartWidth->value(), spinChartHeight->value());
        }
    }
    );
}
colors::colors(QWidget* parent) {
    setParent(parent);

    QFormLayout* lay = new QFormLayout;
    setLayout(lay);

    ColorPickWidget* selPicker = new ColorPickWidget(ROIVert::colors(), this);
    ColorPickWidget* unselPicker = new ColorPickWidget(ROIVert::colors(), this);
    QVector<QColor> chartclrs = { QColor("#222222"),QColor(Qt::lightGray),QColor(Qt::darkGray), QColor(Qt::white)};
    ColorPickWidget* backPicker = new ColorPickWidget(chartclrs, this);
    ColorPickWidget* forePicker = new ColorPickWidget(chartclrs, this);
    ColorPickWidget* gridPicker = new ColorPickWidget(chartclrs, this);

    selPicker->setObjectName("Selected");
    unselPicker->setObjectName("Unselected");
    backPicker->setObjectName("ChartBack");
    forePicker->setObjectName("ChartFore");
    gridPicker->setObjectName("ChartGrid");

    lay->addRow("Selected:", selPicker);
    lay->addRow("Unselected:", unselPicker);
    lay->addRow("Chart Back:", backPicker);
    lay->addRow("Chart Fore:", forePicker);
    lay->addRow("Chart Grid:", gridPicker);

    selPicker->setSelectedColor(ROIVert::colors()[4]);
    unselPicker->setSelectedColor(ROIVert::colors()[3]);
    backPicker->setSelectedColor(chartclrs[0]);
    forePicker->setSelectedColor(chartclrs[1]);
    gridPicker->setSelectedColor(chartclrs[2]);

    connect(selPicker, &ColorPickWidget::colorSelected, this, &colors::setSelectedColor);
    connect(unselPicker, &ColorPickWidget::colorSelected, this, &colors::setUnselectedColor);
    connect(forePicker, &ColorPickWidget::colorSelected, this, &colors::setChartForeColor);
    connect(backPicker, &ColorPickWidget::colorSelected, this, &colors::setChartBackColor);
    connect(gridPicker, &ColorPickWidget::colorSelected, this, &colors::setChartGridColor);
}
QVector<QPair<QString, QColor>> colors::getColors(){
    QVector<QPair<QString, QColor>> ret;

    for (int i = 0; i < layout()->count(); i++) {
        ColorPickWidget* obj = qobject_cast<ColorPickWidget*>(layout()->itemAt(i)->widget());
        if (obj) {
            QPair<QString, QColor> pair;
            pair.first = obj->objectName();
            pair.second = obj->getSelectedColor();
            ret.push_back(pair);
        }
    }
    return ret;
}
void colors::setColors(QVector<QPair<QString, QColor>> clrs){
    for each (QPair<QString, QColor> pair in clrs)
    {
        ColorPickWidget* obj = findChild<ColorPickWidget*>(pair.first);
        if (obj) { obj->setSelectedColor(pair.second); }
    }
}