#pragma once
#include <QObject>

class tMiscSmallWidgets : public QObject
{
    Q_OBJECT

private slots:
    void tSmoothingPickWidget();
    void tSmoothingPickWidget_data();
    void tSmoothingPickWidgetSignal();
    void tRGBWidget();
    void tProjectionPickWidget();
    void tFileIOWidget();
    void tImageDataWindow();
    void tImageSettingsWidget();
};
