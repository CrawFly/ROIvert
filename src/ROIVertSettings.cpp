#include "ROIVertSettings.h"

#include <QSettings>

#include "roivert.h"
#include "dockwidgets/ImageDataWidget.h"
#include "dockwidgets/ImageSettingsWidget.h"
#include "dockwidgets/StyleWidget.h"
#include "dockwidgets/FileIOWidget.h"

struct ROIVertSettings::pimpl {
    Roivert* roivert{ nullptr };
    ImageDataWidget* imagedatawidget{ nullptr };
    ImageSettingsWidget* imagesettingswidget{ nullptr };
    StyleWidget* stylewidget{ nullptr };
    FileIOWidget* fileiowidget{ nullptr };
        
    void savesettings(QSettings& settings) {
        settings.setValue("version", ROIVERTVERSION);
        settings.beginGroup("window");
        settings.setValue("geometry", roivert->saveGeometry());
        settings.setValue("windowState", roivert->saveState());
        settings.endGroup();

        imagedatawidget->saveSettings(settings);
        imagesettingswidget->saveSettings(settings);
        stylewidget->saveSettings(settings);
        fileiowidget->saveSettings(settings);
    }
    void restoresettings(QSettings& settings) {
        
        settings.beginGroup("window");
        roivert->restoreGeometry(settings.value("geometry").toByteArray());
        roivert->restoreState(settings.value("windowState").toByteArray());
        settings.endGroup();

        imagedatawidget->restoreSettings(settings);
        imagesettingswidget->restoreSettings(settings);
        stylewidget->restoreSettings(settings);
        fileiowidget->restoreSettings(settings);
    }
    void resetsettings() {
        // needs bitmask with default reset everything
        roivert->setDefaultGeometry();
        imagedatawidget->resetSettings();
        imagesettingswidget->resetSettings();
        stylewidget->resetSettings();
        fileiowidget->resetSettings();
    }
};


ROIVertSettings::ROIVertSettings(Roivert* r, ImageDataWidget* id, ImageSettingsWidget* is, StyleWidget* s, FileIOWidget* fio) : impl(std::make_unique<pimpl>())
{
    impl->roivert = r;
    impl->imagedatawidget = id;
    impl->imagesettingswidget = is;
    impl->stylewidget = s;
    impl->fileiowidget = fio;
}
ROIVertSettings::~ROIVertSettings() = default;

void ROIVertSettings::saveSettings() { 
    
    QSettings settings;
    impl->savesettings(settings);
}
void ROIVertSettings::restoreSettings() {
    QSettings settings;
    impl->restoresettings(settings);
}
void ROIVertSettings::resetSettings() {
    impl->resetsettings();
}
