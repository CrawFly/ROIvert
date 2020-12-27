#include "ColormapPickWidget.h"
#include <QFormLayout>

namespace {
    std::vector<QPixmap> getColormapPixmaps(std::vector<cv::ColormapTypes> CmapTypes) {

        // Generate some base colormap data
        cv::Mat cv_cmap(10, 255, CV_8U);
        for (int col = 0; col < cv_cmap.size().width; col++) {
            for (int row = 0; row < cv_cmap.size().height; row++) {
                cv_cmap.at<unsigned char>(row, col) = col;
            }
        }

        // first map is always b/w
        std::vector<QPixmap> res;
        res.push_back(QPixmap::fromImage(QImage(cv_cmap.data, cv_cmap.size().width, cv_cmap.size().height, cv_cmap.step, QImage::Format_Grayscale8)));

        for each (auto map in CmapTypes) {
            cv::Mat thismap(cv_cmap.size(), cv_cmap.type());
            cv::applyColorMap(cv_cmap, thismap, map);
            res.push_back(QPixmap::fromImage(QImage(thismap.data, thismap.size().width, thismap.size().height, thismap.step, QImage::Format_BGR888)));
        }

        return res;
    }
}

ColormapPickWidget::ColormapPickWidget(QWidget* parent) noexcept : QWidget(parent) {

    QVBoxLayout* lay = new QVBoxLayout;

    std::vector<QPixmap> c = getColormapPixmaps(cmaps);
    for each (auto mapimage in c) {
        cmbColormap->addItem("");
        cmbColormap->setItemData(cmbColormap->count() - 1, mapimage, Qt::DecorationRole);
    }

    /*
    for (int i = 0; i < c.size(); i++) {
        cmbColormap->addItem("");
        cmbColormap->setItemData(i, c[i], Qt::DecorationRole);
    }
    */

    QSize sz;
    sz.setWidth(255);
    sz.setHeight(20);
    cmbColormap->setIconSize(sz);

    lay->addWidget(cmbColormap);
    //lay->addRow(tr("Colormap:"), cmbColormap);
    setLayout(lay);

    connect(cmbColormap, QOverload<int>::of(&QComboBox::activated), this, &ColormapPickWidget::colormapChanged);
}
int ColormapPickWidget::getColormap() {
    // Note that the int returned here is an enum:
    //  if it's -1 that means don't colormap
    //  otherwise it means use the enum
    const int cmapIndex = cmbColormap->currentIndex();
    if (cmapIndex == 0) { return(-1); }
    else { return(cmaps[cmapIndex - 1]); }
}

void ColormapPickWidget::setColormap(int cmapID) noexcept { 
    // if cmapID matches one of our enums we set it, set the combo box
    size_t cmbInd = 0;
    for (size_t i = 0; i < cmaps.size(); i++) {
        if (cmapID == cmaps[i]) {
            // note that the first index of the combo is b/w
            cmbInd = i + 1;
            break;
        }
    }
    if (cmbInd < cmbColormap->count()) {
        cmbColormap->setCurrentIndex(cmbInd);
    }
}
