#pragma once

#include <QtWidgets/QMainWindow>
#include "roivertcore.h"

class QStringList;

class Roivert : public QMainWindow
{
    Q_OBJECT

public:
    Roivert(QWidget* parent = Q_NULLPTR);
    ~Roivert();
    void setDefaultGeometry();
    QSize getScreenSize() const;

public slots:
    void loadVideo(std::vector<std::pair<QString,size_t>> filenameframelist, const double frameRate, const int dsTime, const int dsSpace);
    void changeFrame(const size_t frame);
    void imgSettingsChanged(ROIVert::imgsettings settings);
    void setInitialSettings(bool restore = true);

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;

    void doConnect();
    void updateContrastWidget(bool isDff);
};
