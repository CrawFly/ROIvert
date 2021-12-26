#pragma once
#include "DockWidgetWithSettings.h"

class FileIOWidget : public DockWidgetWithSettings
{
    Q_OBJECT

public:
    FileIOWidget(QWidget* parent = nullptr);
    ~FileIOWidget();

    void setContentsEnabled(bool);

    void saveSettings(QSettings& settings) const override;
    void restoreSettings(QSettings& settings) override;
    void resetSettings() override;

    bool testoverride = false;

signals:
    void exportTraces(QString filename, bool doHeader, bool doTimeCol);
    void exportROIs(QString filename);
    void importROIs(QString filename);
    void exportCharts(QString filename, int width, int height, int quality, bool ridge);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
