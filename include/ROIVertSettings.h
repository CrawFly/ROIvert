#pragma once
#include <memory>
class Roivert;
class ImageDataWidget; 
class ImageSettingsWidget;
class StyleWidget;
class FileIOWidget;


class ROIVertSettings
{
public:
    ROIVertSettings(Roivert*, ImageDataWidget*, ImageSettingsWidget*, StyleWidget*, FileIOWidget*);
    ~ROIVertSettings();

    void saveSettings();
    void restoreSettings();
    void resetSettings();
    

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};

