#pragma once
#include <QAbstractTableModel>

class ImageDataTableModel : public QAbstractTableModel
{
public:
    explicit ImageDataTableModel(QObject *parent = nullptr);
    ~ImageDataTableModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex getIndexFromName(QString filename);

public slots:
    void setFileModelIndex(const QModelIndex &index, const QModelIndex&);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
