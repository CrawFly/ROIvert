#pragma once
#include <QDialog>
#include "Settingsable.h"

class DialogWithSettings : public QDialog, public Settingsable
{
public:
    DialogWithSettings(QWidget *parent = nullptr, Qt::WindowFlags flags = {0});
};
