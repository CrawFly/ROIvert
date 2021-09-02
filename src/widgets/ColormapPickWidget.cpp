#include "widgets/ColormapPickWidget.h"

#include <QComboBox>
#include <QFormLayout>
#include "opencv2/opencv.hpp"

namespace
{
    std::vector<QPixmap> getColormapPixmaps(std::vector<cv::ColormapTypes> CmapTypes)
    {

        // Generate some base colormap data
        cv::Mat cv_cmap(15, 255, CV_8U);
        for (int col = 0; col < cv_cmap.size().width; col++)
        {
            for (int row = 0; row < cv_cmap.size().height; row++)
            {
                cv_cmap.at<unsigned char>(row, col) = col;
            }
        }

        // first map is always b/w
        std::vector<QPixmap> res;
        res.push_back(QPixmap::fromImage(QImage(cv_cmap.data, cv_cmap.size().width, cv_cmap.size().height, cv_cmap.step, QImage::Format_Grayscale8)));

        for (auto &map : CmapTypes)
        {
            cv::Mat thismap(cv_cmap.size(), cv_cmap.type());
            cv::applyColorMap(cv_cmap, thismap, map);
            res.push_back(QPixmap::fromImage(QImage(thismap.data, thismap.size().width, thismap.size().height, thismap.step, QImage::Format_BGR888)));
        }

        return res;
    }
    const std::vector<cv::ColormapTypes> cmaps{cv::COLORMAP_DEEPGREEN, cv::COLORMAP_HOT, cv::COLORMAP_INFERNO, cv::COLORMAP_PINK, cv::COLORMAP_BONE, cv::COLORMAP_TURBO, cv::COLORMAP_TWILIGHT, cv::COLORMAP_OCEAN };
}

ColormapPickWidget::ColormapPickWidget(QWidget *parent) : QWidget(parent)
{
    auto lay{std::make_unique<QVBoxLayout>()};
    cmbColormap = std::make_unique<QComboBox>();
    cmbColormap->setToolTip(tr("Choose a colormap for displaying the video"));

    std::vector<QPixmap> c = getColormapPixmaps(cmaps);
    for (auto &mapimage : c)
    {
        cmbColormap->addItem("");
        cmbColormap->setItemData(cmbColormap->count() - 1, mapimage, Qt::DecorationRole);
    }

    QSize sz;
    sz.setWidth(220);
    sz.setHeight(20);
    cmbColormap->setIconSize(sz);

    lay->addWidget(cmbColormap.get());
    setLayout(lay.release());

    connect(cmbColormap.get(), QOverload<int>::of(&QComboBox::activated), this, &ColormapPickWidget::colormapChanged);
}
int ColormapPickWidget::getColormap() const
{
    // If return is -1 that means don't colormap
    // Otherwise interpret as enum
    const int cmapIndex = cmbColormap->currentIndex();
    if (cmapIndex == 0)
    {
        return (-1);
    }
    else
    {
        const size_t ind = cmapIndex - 1;
        return (cmaps[ind]);
    }
}

void ColormapPickWidget::setColormap(int cmapID)
{
    // if cmapID matches one of our enums we set it, set the combo box
    size_t cmbInd = 0;
    for (size_t i = 0; i < cmaps.size(); i++)
    {
        if (cmapID == cmaps[i])
        {
            // note that the first index of the combo is b/w
            cmbInd = i + 1;
            break;
        }
    }
    if (cmbInd < cmbColormap->count())
    {
        cmbColormap->setCurrentIndex(cmbInd);
    }
}
