/**
 * \class  ProjectionPickWidget.h
 *
 * \brief  Simple buttongroup widget for selecting projection (none/mean/min/max)
 *
 * \author neuroph
*/
#pragma once

#include <QWidget>
#include <QComboBox>
#include "opencv2/opencv.hpp"


class ColormapPickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColormapPickWidget(QWidget* parent = nullptr) noexcept;
    /**
     * @brief get colormap id
     * @return The int returned is the opencv enum, or -1 which indicates no colormapping
    */
    int getColormap();

    /**
     * @brief set the current colormap
     * @param cmapID a cv colormap enum in the list of statically defined colormaps, or -1 which indicates no colormapping
    */
    void setColormap(int cmapID) noexcept;

signals:
    void colormapChanged();

private:
    // consider whether these go in core?
    const std::vector<cv::ColormapTypes> cmaps{ cv::COLORMAP_DEEPGREEN , cv::COLORMAP_HOT , cv::COLORMAP_INFERNO, cv::COLORMAP_PINK, cv::COLORMAP_BONE };
    QComboBox* cmbColormap = new QComboBox;
};

