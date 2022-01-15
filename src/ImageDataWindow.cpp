#include "ImageDataWindow.h"

#include "ImageDataTableModel.h"

#include <QIcon>
#include <QBoxLayout>
#include <QGridLayout>
#include <QTableView>
#include <QTreeView>
#include <QFileSystemModel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QHeaderView>

struct ImageDataWindow::pimpl
{
};




ImageDataWindow::ImageDataWindow(QWidget* parent) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint), impl(std::make_unique<pimpl>())
{
    setModal(true);
    setWindowTitle("Load Dataset");
    setWindowIcon(QIcon(":/icons/GreenCrown.png"));

    auto lay = new QGridLayout(this);

    auto folderview = new QTreeView();
    lay->addWidget(folderview, 0, 0, 2, 1);

    auto fileview = new QTableView();
    lay->addWidget(fileview, 0, 1);

    auto buttonlay = new QHBoxLayout();
    lay->addLayout(buttonlay, 1, 1);
    buttonlay->addWidget(new QPushButton("Cancel"));
    buttonlay->addStretch(0);
    buttonlay->addWidget(new QPushButton("Load"));


    QFileSystemModel* model = new QFileSystemModel; // todo: stored path
    model->setRootPath(QDir::currentPath());
    model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    model->setOption(QFileSystemModel::DontUseCustomDirectoryIcons, true);
    folderview->setModel(model);
    folderview->setAnimated(false);
    for (int i = 1; i < model->columnCount(); ++i) {
        folderview->hideColumn(i);
    }

    
    auto imagemodel= new ImageDataTableModel;
    auto proxyModel = new QSortFilterProxyModel;
    proxyModel->setSourceModel(imagemodel);
    fileview->setModel(proxyModel);
    fileview->setSortingEnabled(true);
    fileview->sortByColumn(0,Qt::SortOrder::AscendingOrder);
    fileview->verticalHeader()->setVisible(false);
    connect(folderview->selectionModel(), &QItemSelectionModel::currentChanged, imagemodel, &ImageDataTableModel::setFileModelIndex);

}

ImageDataWindow::~ImageDataWindow() {
}