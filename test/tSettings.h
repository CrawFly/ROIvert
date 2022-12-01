#pragma once
#include <QObject>
class ROIVertSettings;
class Roivert;

class tSettings : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void tsavesettings();
    void tresetsettings();
    void trestoresettings();

private:
    Roivert* r;
    ROIVertSettings* rsettings;

    void applyCustomValuesToWidgets();
    void checkWidgetValuesDefault();
    void checkWidgetValuesCustom();
};
