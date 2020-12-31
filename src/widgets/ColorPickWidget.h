/**
 * \class  ColorPickWidget.h
 *
 * \brief  Widget for selectiong some static and one dynamic color
 *
 * \author neuroph
*/
#pragma once

#include <QWidget>

class QColorDialog;
class QButtonGroup;

class ColorPickWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief ColorPickWidget
     * @param clrs a list of static colors to use
    */
    explicit ColorPickWidget(QVector<QColor> clrs, QWidget* parent = nullptr);

    /**
     * @brief Get the currently selected color
     * @return The selected color
    */
    QColor getSelectedColor() const;

    /**
     * @brief Set The currently selected color (using dynamic color if not in list)
     * @param clr The color to set.
    */
    void setSelectedColor(QColor clr);

signals:
    /**
     * @brief Signal that fires when a color is selected
    */
    void colorSelected(QColor clr);

private:
    void setColors(QVector<QColor> clrs);
    QVector<QColor> colors;
    QColor customColor = QColor("#FFFFFF");
    QButtonGroup* grp;
    int selectedind = 0;

    void setCustomColor();
    QColorDialog* clrDlg;

    static const QString getStyleSheet(QColor clr);
};
