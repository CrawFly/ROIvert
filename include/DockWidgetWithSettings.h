#pragma once
#include <QDockWidget>
#include "Settingsable.h"

class DockWidgetWithSettings : public QDockWidget, public Settingsable
{
public:
    DockWidgetWithSettings(QWidget *parent = nullptr);
};
