#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include <QColor>
#include <QButtonGroup>
#include <QColorDialog>

class ColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPicker(QWidget* parent = nullptr);
    explicit ColorPicker(QVector<QColor> clrs, QWidget* parent = nullptr);
    ~ColorPicker();

    void setColors(QVector<QColor> clrs);
    QColor getSelectedColor() const;
    void setSelectedColor(QColor clr);

signals:
    void colorSelected(QColor clr);

private:
    QVector<QColor> colors;
    QColor customColor = QColor("#FFFFFF");
    QButtonGroup* grp = new QButtonGroup;
    int selectedind = 0;

    void setCustomColor();
    QColorDialog* clrDlg = new QColorDialog;

    static const QString getStyleSheet(QColor clr);
};
