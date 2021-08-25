#include "dockwidgets/ImageDataWidget.h"

#include <QBoxLayout>
#include <QCompleter>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStyleOption>

struct ImageDataWidget::pimpl {
    QWidget* contents = new QWidget;

    QLineEdit* txtFilePath = new QLineEdit();
    QPushButton* cmdBrowseFilePath = new QPushButton();
    QDoubleSpinBox* spinFrameRate = new QDoubleSpinBox();
    QSpinBox* spinDownTime = new QSpinBox();
    QSpinBox* spinDownSpace = new QSpinBox();
    QLabel* lblFileInfo = new QLabel();
    QProgressBar* progBar = new QProgressBar();
    QPushButton* cmdLoad = new QPushButton();
    
    QCompleter completer;
    QFileSystemModel fsmodel;

    void init() {
        {
            fsmodel.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
            fsmodel.setRootPath(QDir::currentPath());
        }
        {
            completer.setModel(&fsmodel);
            completer.setCompletionMode(QCompleter::InlineCompletion);
        }
        {
            txtFilePath->setCompleter(&completer);
            txtFilePath->setMinimumWidth(100);
            txtFilePath->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Preferred);
            txtFilePath->setToolTip(tr("Input a path to a folder that contains a dataset (i.e. tiff files)"));
        }
        {
            cmdBrowseFilePath->setText("...");
            const QSize textSize = cmdBrowseFilePath->fontMetrics().size(Qt::TextShowMnemonic, cmdBrowseFilePath->text());
            QStyleOptionButton opt;
            opt.initFrom(cmdBrowseFilePath);
            opt.rect.setSize(textSize);
            cmdBrowseFilePath->setFixedSize(cmdBrowseFilePath->style()->sizeFromContents(QStyle::CT_PushButton, &opt, textSize, cmdBrowseFilePath));
            cmdBrowseFilePath->setToolTip(tr("Open a dialog to select a folder"));
        }
        {
            spinFrameRate->setValue(30);
            spinFrameRate->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
            spinFrameRate->setMaximum(999);
            spinFrameRate->setMinimum(1);
            const QSize textSize = cmdBrowseFilePath->fontMetrics().size(Qt::TextShowMnemonic, "999");
            spinFrameRate->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            spinFrameRate->setToolTip(tr("Set the frame rate in which the data were recorded\nin frames per second (FPS)"));
        }
        {
            spinDownTime->setMinimum(1);
            spinDownTime->setMaximum(100);
            spinDownTime->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            spinDownTime->setToolTip(tr("Import only every nth frame\nFrame rate should reflect the raw value"));
        }
        {
            spinDownSpace->setMinimum(1);
            spinDownSpace->setMaximum(100);
            spinDownSpace->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            spinDownSpace->setToolTip(tr("Import only every nth pixel"));
        }
        {
            cmdLoad->setText("Load Files");
            cmdLoad->setEnabled(false);
        }
        {
            progBar->setMaximum(100);
            progBar->setVisible(false);
        }
    }

    void layout() {
        QVBoxLayout* vlay = new QVBoxLayout();
        contents->setLayout(vlay);
        QFormLayout* formlay = new QFormLayout();
        vlay->addLayout(formlay);
        
        {
            QWidget *wid = new QWidget;
            QHBoxLayout *lay = new QHBoxLayout;
            lay->addWidget(txtFilePath);
            lay->addWidget(cmdBrowseFilePath);
            lay->setAlignment(Qt::AlignLeft);
            lay->setContentsMargins(0, 0, 0, 0);
            formlay->addRow(tr("File Path:"), lay);
        }

        formlay->addRow(tr("Frame Rate:"), spinFrameRate);
        formlay->addRow(tr("Frame Subset:"), spinDownTime);
        formlay->addRow(tr("Pixel Subset:"), spinDownSpace);

        vlay->addWidget(cmdLoad);
        vlay->addWidget(progBar);
        vlay->addStretch();
    }

    void browse(ImageDataWidget* par) {
        QString initpath = QDir::currentPath();
        QString currpath = txtFilePath->text();
        if (!currpath.isEmpty() && QFileInfo(QDir(currpath).path()).isDir()) {
            initpath = currpath; 
        }
        QString fileName = QFileDialog::getOpenFileName(par,
                                                        tr("Select a Tiff File in the Dataset"), 
                                                        initpath, 
                                                        tr("Tiff Files (*.tif *.tiff)"));
        
        // todo: check here if the file is multipage?
        if (!fileName.isEmpty()) { 
            txtFilePath->setText(QFileInfo(fileName).absolutePath()); 
        }

    }
    
    void filePathChanged(const QString& filepath) {
        
        cmdLoad->setEnabled(false);

        if (!QFileInfo(filepath).isDir())
        {
            return;
        }

        QStringList fl = QDir(filepath).entryList(QStringList() << "*.tif" << "*.tiff", QDir::Files);
        if (fl.size() > 0) { 
            cmdLoad->setEnabled(true); 
        }
    }
    void load(ImageDataWidget* par) {
        QFileInfoList fileinfolist = QDir(txtFilePath->text()).entryInfoList(QStringList() << "*.tif" << "*.tiff", QDir::Files);
        QStringList filelist;
        for (auto &filename:fileinfolist)
        {
            filelist << filename.absoluteFilePath();
        }
        emit par->fileLoadRequested(filelist, spinFrameRate->value(), spinDownTime->value(), spinDownSpace->value());
    }
};

ImageDataWidget::ImageDataWidget(QWidget* parent) : QDockWidget(parent) {
    this->setWidget(impl->contents);
    impl->init();
    impl->layout();

    
    connect(impl->cmdBrowseFilePath, &QPushButton::clicked, this, [=] { impl->browse(this); });
    connect(impl->txtFilePath, &QLineEdit::textChanged, this, [=](const QString& filepath) { impl->filePathChanged(filepath); });
    connect(impl->cmdLoad, &QPushButton::clicked, this, [=] { impl->load(this); });
    connect(impl->spinFrameRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ImageDataWidget::frameRateChanged);
}

void ImageDataWidget::setContentsEnabled(bool onoff) {
    impl->contents->setEnabled(onoff);
}

void ImageDataWidget::setProgBar(int val) {
    if (val >= 0 && val <= impl->progBar->maximum()) {
        impl->progBar->setVisible(true);
        impl->progBar->setValue(val);
        return;
    }
    impl->progBar->setVisible(false);
}
