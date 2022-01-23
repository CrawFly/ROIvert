#include "ImageLoadingProgressWindow.h"
#include <QBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QApplication>
//#include <QGraphicsView>

struct ImageLoadingProgressWindow::pimpl
{
    QVBoxLayout lay;
    std::vector<QProgressBar *> progbars;
    std::vector<QLabel *> proglbls;

    void init()
    {
        std::vector<QString> lbls = {"Reading files from disk.", "Calculating histogram.", "Calculating df/f."};
        for (size_t i = 0; i < 3; ++i)
        {
            auto lbl = new QLabel;
            lbl->setText(tr(lbls[i].toStdString().c_str()));
            lbl->setEnabled(false);
            auto bar = new QProgressBar;
            bar->setMinimum(0);
            bar->setMaximum(100);

            proglbls.push_back(lbl);
            progbars.push_back(bar);
            lay.addWidget(lbl);
            lay.addWidget(bar);
        }
    }
};

ImageLoadingProgressWindow::ImageLoadingProgressWindow(QWidget *parent) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint), impl(std::make_unique<pimpl>())
{
    setModal(true);
    impl->init();
    setLayout(&impl->lay);
}

ImageLoadingProgressWindow::~ImageLoadingProgressWindow()
{
}

void ImageLoadingProgressWindow::setProgress(int level, int value)
{
    impl->progbars[level]->setValue(value);
    if (!impl->proglbls[level]->isEnabled())
    {
        for (auto lbl : impl->proglbls)
        {
            lbl->setEnabled(false);
        }
        impl->proglbls[level]->setEnabled(true);
    }

    qApp->processEvents();
    qApp->processEvents();
}