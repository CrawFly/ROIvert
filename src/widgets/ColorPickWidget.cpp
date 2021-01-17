#include "widgets/ColorPickWidget.h"
#include <QBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QButtonGroup>
#include <QColorDialog>

#include <qdebug.h>

ColorPickWidget::ColorPickWidget(QVector<QColor> clrs, QWidget* parent) : QWidget(parent)
{
    grp = new QButtonGroup;
    clrDlg = new QColorDialog;
    QHBoxLayout* lay = new QHBoxLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSizeConstraint(QLayout::SetFixedSize);
    lay->setSpacing(0);
    setLayout(lay);
    setColors(clrs);
}

void ColorPickWidget::setColors(QVector<QColor> clrs) {
    // check how many buttons we have
    colors = clrs;

    const int nbuttons = layout()->count();
    const int ncolors = clrs.size();

    for (int i = nbuttons; i < ncolors + 1; i++) {
        // add buttons:
        layout()->addWidget(new QPushButton);
    }
    for (int i = ncolors + 1; i < nbuttons; i++) {
        delete(layout()->itemAt(i)->widget());
    }


    // Set style Sheet for fixed colors:
    for (int i = 0; i < ncolors; ++i) {
        QPushButton* t = qobject_cast<QPushButton*>(layout()->itemAt(i)->widget());
        if (t) {
            
            t->setStyleSheet(getStyleSheet(clrs[i]));
            t->setCheckable(true);
            t->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            grp->addButton(t, i);

        }
    }

    {
        QPushButton* t(qobject_cast<QPushButton*>(layout()->itemAt(ncolors)->widget()));
        if (t) {
            t->setCheckable(true);
            QMenu* mnu = new QMenu;
            mnu->addAction("Set Custom Color", this, &ColorPickWidget::setCustomColor);
            t->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            t->setStyleSheet(getStyleSheet(customColor));
            t->setMenu(mnu);
            grp->addButton(t, ncolors);
        }
    }
    connect(grp, &QButtonGroup::idClicked, this, [=](int id) {emit colorSelected(colors[id]); });
}
void ColorPickWidget::setCustomColor() {
    
    const QColor clr = clrDlg->getColor(customColor);
    if (clr.isValid()) {
        customColor = clr;
        QPushButton* t(qobject_cast<QPushButton*>(layout()->itemAt(layout()->count() - 1)->widget()));
        if (t) {
            t->setStyleSheet(getStyleSheet(customColor));
            t->setChecked(true);
            emit colorSelected(clr);
        }
    }

}
QColor ColorPickWidget::getSelectedColor() const {
    const int id = grp->id(grp->checkedButton());
    if (id < colors.size()) {
        return colors[id];
    }
    return customColor;
}
void ColorPickWidget::setSelectedColor(QColor clr) {
    const auto ind = colors.indexOf(clr);
    if(ind>-1){
        selectedind = ind;
        QPushButton* t(qobject_cast<QPushButton*>(layout()->itemAt(ind)->widget()));
        if (t) { t->setChecked(true); }
    }
    else {
        selectedind = colors.size(); // Custom color selected
        customColor = clr;
        QPushButton* t(qobject_cast<QPushButton*>(layout()->itemAt(layout()->count() - 1)->widget()));
        if (t) {
            t->setStyleSheet(getStyleSheet(customColor));
            t->setChecked(true);
        }
    }
    emit colorSelected(clr);
}
const QString ColorPickWidget::getStyleSheet(QColor clr){
    QString backclr = clr.name();
    QString bordclr = "black";
    if (clr.lightness() < 50) {
        bordclr = "red";
    }
    return "QPushButton { border: 0px; background-color: " + backclr + "; max-width: 30; max-height: 30; min-width: 30; min-height: 30;} QPushButton:checked { border: 4px solid " + bordclr + "; max-width: 30; max-height: 30; min-width: 30; min-height: 30; }";
}