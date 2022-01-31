#include "DialogWithSettings.h"

DialogWithSettings::DialogWithSettings(QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags)
{
    this->setLayout(&toplay);
}
