#include "ImageDataTableModel.h"

#include <QDateTime>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QFont>
#include <QFontMetrics>

#include "tinytiffreader.h"

#include <QDebug>
#include <QProgressDialog>
#include <QApplication>


struct TiffMeta {
    // store everything in a stringlist, note that data are retrieved columnwise, so these should be adjacent
    QStringList name;
    QStringList date;
    QStringList frames;
    QStringList width;
    QStringList height;

    void clear() {
        name.clear();
        date.clear();
        frames.clear();
        width.clear();
        height.clear();
    }

    void reserve(int sz) {
        name.reserve(sz);
        date.reserve(sz);
        frames.reserve(sz);
        width.reserve(sz);
        height.reserve(sz);
    }

    void push_back(QString fn, QDateTime fd, int fr, int w, int h) {
        name.push_back(fn);
        date.push_back(fd.toString("yyyy.MM.dd hh:mm:ss"));
        frames.push_back(QString::number(fr));
        width.push_back(QString::number(w));
        height.push_back(QString::number(h));
    }

    size_t size() {
        return name.size();
    }
};

struct ImageDataTableModel::pimpl
{
    QStringList colnames = { "Name", "Date", "Frames", "Width", "Height"};

    TiffMeta imagedata;
    
    void retrieveFileData(QDir fp) {
        QList<QFileInfo> files = fp.entryInfoList({"*.tif", "*.tiff"}, QDir::Files, QDir::Name);
        imagedata.clear();
        imagedata.reserve(files.size());

        auto cntr = 0;
        QProgressDialog progress("Processing Files In Folder", "", 0, 100);
        if (files.size() > 10) {
            progress.setWindowModality(Qt::WindowModal);
            progress.setCancelButton(nullptr);
            progress.setWindowIcon(QIcon(":/icons/GreenCrown.png"));
            progress.show();
        }

        for (auto& file : files) {
            int frames=0, width=0, height=0;
            
            auto tiff = TinyTIFFReader_open(file.absoluteFilePath().toStdString().c_str());
            if (tiff) {
                frames = TinyTIFFReader_countFrames(tiff);
                width = TinyTIFFReader_getWidth(tiff);
                height = TinyTIFFReader_getHeight(tiff);
            }
            TinyTIFFReader_close(tiff);
            imagedata.push_back(file.fileName(), file.lastModified(), frames, width, height);
            progress.setValue((100 * cntr++) / files.size());
            qApp->processEvents();
        }
        progress.reset();
    }

    QString getData(const QModelIndex& index){
        
        switch (index.column()) {
        case 0:
            return imagedata.name[index.row()];
        case 1:
            return imagedata.date[index.row()];
        case 2:
            return imagedata.frames[index.row()];
        case 3:
            return imagedata.width[index.row()];
        case 4:
            return imagedata.height[index.row()];
        default:
            return "";
        }
    
    }
};

ImageDataTableModel::ImageDataTableModel(QObject* parent) : QAbstractTableModel(parent), impl(std::make_unique<pimpl>()) { }
ImageDataTableModel::~ImageDataTableModel() { }

int ImageDataTableModel::rowCount(const QModelIndex& parent) const { 
    return(impl->imagedata.size()); 
}

int ImageDataTableModel::columnCount(const QModelIndex& parent) const { 
    return 5;
}
QVariant ImageDataTableModel::data(const QModelIndex& index, int role) const { 
    switch (role)
    {
    case Qt::DisplayRole:
        return impl->getData(index);
    case Qt::SizeHintRole:
        return QVariant();

    case Qt::FontRole:
        return QFont();
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter | Qt::AlignLeft;
    default:
        return QVariant();
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

    auto model = dynamic_cast<const QFileSystemModel*>(index.model());
    auto filepath = QDir(model->fileInfo(index).absoluteFilePath());

    impl->retrieveFileData(filepath);

    endResetModel();
}

QModelIndex ImageDataTableModel::getIndexFromName(QString filename) {
    for (size_t i = 0; i < impl->imagedata.size(); ++i) {
        if (impl->imagedata.name[i] == filename) {
            return createIndex(i, 0);
        }
    }
    return QModelIndex();
}