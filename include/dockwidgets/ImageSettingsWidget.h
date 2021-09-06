#pragma once
#include "DockWidgetWithSettings.h"
#include "roivertcore.h"

class DisplaySettings;

class ImageSettingsWidget : public DockWidgetWithSettings
{
    Q_OBJECT
    public:
        ImageSettingsWidget(QWidget* parent = nullptr, DisplaySettings* = nullptr);
        ~ImageSettingsWidget();
        void setHistogram(std::vector<float> &data);
        void setContrast(ROIVert::contrast c);
        void setContentsEnabled(bool);

        void saveSettings(QSettings& settings) const override;
        void restoreSettings(QSettings& settings) override;
        void resetSettings() override;

        bool isProjectionActive() const;

    signals:
        void imgSettingsChanged(ROIVert::imgsettings newsettings);
        void dffToggled(bool);
    
    public slots:
        void dffToggle(bool);

    private:
        struct pimpl;
        std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

