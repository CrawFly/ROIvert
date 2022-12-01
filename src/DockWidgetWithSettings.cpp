#include "DockWidgetWithSettings.h"

DockWidgetWithSettings::DockWidgetWithSettings(QWidget *parent) : QDockWidget(parent)
{
    auto contents = new QWidget;
    this->setWidget(contents);
    contents->setLayout(&toplay);
    this->connect(this, &QDockWidget::topLevelChanged, this, &QDockWidget::adjustSize);
}
