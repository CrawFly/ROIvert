#pragma once
#include <QVBoxLayout>
#include <QSettings>

class Settingsable
{
    // base class for dockwidgets and dialogs that have settings
public:
    Settingsable();
    virtual void saveSettings(QSettings &settings) const = 0;
    virtual void restoreSettings(QSettings &settings) = 0;
    virtual void resetSettings() = 0;

    QVBoxLayout toplay;
    bool getSettingsStorage() const;
    void setSettingsStorage(bool);

private:
    QAction *reset;
    QAction *save;
};
