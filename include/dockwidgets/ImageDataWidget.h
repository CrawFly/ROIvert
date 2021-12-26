#pragma once
#include "DockWidgetWithSettings.h"

class QSettings;

class ImageDataWidget : public DockWidgetWithSettings
{
    Q_OBJECT

public:
    ImageDataWidget(QWidget* parent = nullptr);
    ~ImageDataWidget();

    void setContentsEnabled(bool);

    void saveSettings(QSettings& settings) const override;
    void restoreSettings(QSettings& settings) override;
    void resetSettings() override;

signals:
    void fileLoadRequested(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace, bool isfolder);
    void frameRateChanged(double fr);

public slots:
    void setProgBar(int val);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
