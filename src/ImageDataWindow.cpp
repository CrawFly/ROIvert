#include "ImageDataWindow.h"

#include "ImageDataTableModel.h"

#include <QIcon>
#include <QBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTableView>
#include <QTreeView>
#include <QFileSystemModel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSizePolicy>
#include <QSplitter>
#include <QTime>

#include <QDebug>
struct ImageDataWindow::pimpl
{
    void init(QGridLayout* top) {
        top->addWidget(&splitter,0,0);

        splitter.addWidget(&folderview);

        initmodels();
        initviews();
        {
            auto placeholder = new QWidget;
            auto lay = new QVBoxLayout;
            placeholder->setLayout(lay);
            lay->addWidget(&fileview);
            lay->addLayout(makeControlPanel());
            splitter.addWidget(placeholder);
            lay->setContentsMargins(0, 0, 0, 0);
        }
        
        connect(folderview.selectionModel(), &QItemSelectionModel::currentChanged, &immodel, &ImageDataTableModel::setFileModelIndex);
        connect(&immodel, &ImageDataTableModel::modelReset, [=]() {fileview.resizeColumnsToContents();});
        connect(fileview.selectionModel(), &QItemSelectionModel::selectionChanged, [&](const QItemSelection& selected, const QItemSelection) { updateLabel(); });
        connect(&spinFrameRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double) { updateLabel();  });
    }

    void getSelectedData(QStringList& files, std::vector<size_t>& frames) {
        auto inds = fileview.selectionModel()->selectedIndexes();
        files.reserve(inds.length());
        frames.reserve(inds.length());

        for (auto& ind : inds) {
            files.push_back(ind.data(Qt::DisplayRole).toString());
            frames.push_back(ind.siblingAtColumn(2).data(Qt::DisplayRole).toInt());
        }
    }

    QPushButton* getCancelButton() { return &cmdCancel; }
    QPushButton* getLoadButton() { return &cmdLoad; }

    
private:
    void updateLabel() {
        auto inds = fileview.selectionModel()->selectedIndexes();

        // get the total number of frames
        int nframes = 0;
        int width = 0, height = 0;

        for (auto& ind : inds) {
            int w = ind.siblingAtColumn(3).data(Qt::DisplayRole).toInt();
            int h = ind.siblingAtColumn(4).data(Qt::DisplayRole).toInt();
            if (width == 0) {
                width = w;
                height = h;
            }
            if (w != width || h != height) {
                lblDatasetInfo.setText(QString("Width/Height Mismatch: All selected files must have the same width and height"));
                return;
            }

            nframes += ind.siblingAtColumn(2).data(Qt::DisplayRole).toInt();
        }
        int nseconds = nframes/spinFrameRate.value();
        auto minutes = std::floor(nseconds / 60);
        auto seconds = nseconds % 60;

            
        lblDatasetInfo.setText(QString("Files Selected: %1\nFrames: %2\nDuration %3:%4").arg(inds.size()).arg(nframes).arg(minutes).arg(seconds,2,'f',0,'0'));
    }
    QVBoxLayout* makeControlPanel() {

        auto ret = new QVBoxLayout;
        {
            auto lay = new QHBoxLayout;
            lay->addWidget(new QLabel(tr("Frame Rate")));
            lay->addWidget(&spinFrameRate);
            lay->addStretch(0);
            lay->addWidget(new QLabel(tr("Frame Subset")));
            lay->addWidget(&spinDownTime);
            lay->addStretch(0);
            lay->addWidget(new QLabel(tr("Pixel Subset")));
            lay->addWidget(&spinDownSpace);
            ret->addLayout(lay);

            
            spinFrameRate.setValue(30);
            spinFrameRate.setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
            spinFrameRate.setMaximum(999);
            spinFrameRate.setMinimum(1);
            spinFrameRate.setObjectName("spinFrameRate");
            spinFrameRate.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            spinFrameRate.setToolTip(tr("Set the frame rate in which the data were recorded\nin frames per second (FPS)"));
            
            spinDownTime.setMinimum(1);
            spinDownTime.setMaximum(100);
            spinDownTime.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            spinDownTime.setToolTip(tr("Import only every nth frame\nNote: the frame rate should reflect the raw value\nNote: frame subsets for multipage tiff files do not speed up loading"));
            spinDownTime.setObjectName("spinDownTime");

            spinDownSpace.setMinimum(1);
            spinDownSpace.setMaximum(100);
            spinDownSpace.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
            spinDownSpace.setToolTip(tr("Import only every nth pixel"));
            spinDownSpace.setObjectName("spinDownSpace");
        }
        {
            ret->addWidget(&lblDatasetInfo);
            lblDatasetInfo.setText(tr("\n\n\n"));
            lblDatasetInfo.setWordWrap(true);
        }
        {
            auto lay = new QHBoxLayout;
            lay->addStretch(0);
            lay->addWidget(&cmdCancel);
            lay->addWidget(&cmdLoad);
            cmdCancel.setText(tr("Cancel"));
            cmdLoad.setText(tr("Load"));
            cmdCancel.setAutoDefault(false);

            ret->addLayout(lay);
        }
        return ret;
    }
    void initmodels() {
        fsmodel.setRootPath(QDir::currentPath());
        fsmodel.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
        fsmodel.setOption(QFileSystemModel::DontUseCustomDirectoryIcons, true);

        immodel_proxy.setSourceModel(&immodel);

    }
    void initviews() {
        folderview.setModel(&fsmodel);
        folderview.setAnimated(false);
        for (int i = 1; i < fsmodel.columnCount(); ++i) {
            folderview.hideColumn(i);
        }
        
        fileview.setModel(&immodel_proxy);
        fileview.setSortingEnabled(true);
        fileview.sortByColumn(0,Qt::SortOrder::AscendingOrder);
        fileview.verticalHeader()->setVisible(false);
    }


    QSplitter splitter;
    QTreeView folderview;
    QTableView fileview;
    QFileSystemModel fsmodel;
    ImageDataTableModel immodel;
    QSortFilterProxyModel immodel_proxy;


    QHBoxLayout controlpanel;
    QDoubleSpinBox spinFrameRate;
    QSpinBox spinDownSpace;
    QSpinBox spinDownTime;
    QLabel lblDatasetInfo;

    QPushButton cmdCancel;
    QPushButton cmdLoad;
    
};

ImageDataWindow::ImageDataWindow(QWidget* parent) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint), impl(std::make_unique<pimpl>())
{
    
    setModal(true);
    setWindowTitle("Load Dataset");
    setWindowIcon(QIcon(":/icons/GreenCrown.png"));

    
    auto toplay = new QGridLayout(this);
    impl->init(toplay);

    connect(impl->getCancelButton(), &QPushButton::pressed, this, &QDialog::reject);

    connect(impl->getLoadButton(), &QPushButton::pressed, [&] {
        QStringList filelist;
        std::vector<size_t> frames;
        impl->getSelectedData(filelist, frames);
        emit fileLoadRequested(filelist, frames);
        hide();
    });
}

ImageDataWindow::~ImageDataWindow() {
}