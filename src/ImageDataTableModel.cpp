#include "ImageDataTableModel.h"

#include <QDateTime>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QFont>



struct ImageDataTableModel::pimpl
{
    QList<QFileInfo> files;
    QStringList colnames = { "AAA", "BBB" };
};

ImageDataTableModel::ImageDataTableModel(QObject* parent) : QAbstractTableModel(parent), impl(std::make_unique<pimpl>()) { }
ImageDataTableModel::~ImageDataTableModel() { }

int ImageDataTableModel::rowCount(const QModelIndex& parent) const { 
    return(impl->files.size()); 
}

int ImageDataTableModel::columnCount(const QModelIndex& parent) const { 
    return 2;
}
QVariant ImageDataTableModel::data(const QModelIndex& index, int role) const { 
    if(role==Qt::TextAlignmentRole){
        return Qt::AlignVCenter;
    }
    else if(role == Qt::FontRole){
        return QFont();
    }
    else if(role == Qt::CheckStateRole){
        return QVariant();
    }

    switch (index.column()) {
    case 0:
        return impl->files[index.row()].fileName();
        break;
    case 1:
        return impl->files[index.row()].lastModified();
        break;
    }
    return QVariant(); 
}
Qt::ItemFlags ImageDataTableModel::flags(const QModelIndex &index) const { 
    switch (index.column()) {
    case 0:
        return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
        break;
    case 1:
        return Qt::ItemFlag::ItemIsEnabled;
        break;
    }
    return Qt::NoItemFlags;
}
QVariant ImageDataTableModel::headerData(int section, Qt::Orientation orientation, int role) const { 
    if(role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal){
        return impl->colnames[section];
    }
    return QVariant(); 
}
void ImageDataTableModel::setFileModelIndex(const QModelIndex &index, const QModelIndex&){ 
    beginResetModel();
    auto model = (QFileSystemModel*)index.model();
    auto fp = model->fileInfo(index).absoluteFilePath();
    auto qd = QDir(fp);
    impl->files = qd.entryInfoList({"*.tif", "*.tiff"}, QDir::Files, QDir::Name);
    endResetModel();
}

