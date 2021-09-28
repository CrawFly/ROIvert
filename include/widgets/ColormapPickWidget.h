/**
 * \class  ProjectionPickWidget.h
 *
 * \brief  Simple buttongroup widget for selecting projection (none/mean/min/max)
 *
 * \author neuroph
*/
#pragma once

#include <QWidget>
//class QComboBox;
#include <QComboBox>

class ColormapPickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColormapPickWidget(QWidget* parent = nullptr);
    /**
     * @brief get colormap id
     * @return The int returned is the opencv enum, or -1 which indicates no colormapping
    */
    int getColormap() const;

    /**
     * @brief set the current colormap
     * @param cmapID a cv colormap enum in the list of statically defined colormaps, or -1 which indicates no colormapping
    */
    void setColormap(int cmapID);

signals:
    void colormapChanged();

private:
    std::unique_ptr<QComboBox> cmbColormap{ nullptr };
};

