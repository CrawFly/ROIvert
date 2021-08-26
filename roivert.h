#pragma once

#include <QtWidgets/QMainWindow>
#include "roivertcore.h"

class QStringList;

class Roivert : public QMainWindow
{
    Q_OBJECT

public:
    Roivert(QWidget *parent = Q_NULLPTR);
    ~Roivert();
    void restoreSettings();

public slots:
    void loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace);
    void changeFrame(const size_t frame);
    void imgSettingsChanged(ROIVert::imgsettings settings);
    
protected:
    void closeEvent(QCloseEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();

    void doConnect();
    void frameRateChanged(double frameRate);
    void updateContrastWidget(bool isDff);
    void resetSettings();

};
