#pragma once
#include <QDialog>

namespace cv
{
    class Mat;
}
class ImageLoadingProgressWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ImageLoadingProgressWindow(QWidget *parent = nullptr);
    ~ImageLoadingProgressWindow();

public slots:
    void setProgress(const int level, const int value);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};