#pragma once
#include <QDialog>
class ImageDataWindow : public QDialog 
{
Q_OBJECT

public: 
    explicit ImageDataWindow(QWidget* parent = nullptr);
    ~ImageDataWindow();

signals:
    void fileLoadRequested(QStringList filelist, std::vector<size_t> framesperfile);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
