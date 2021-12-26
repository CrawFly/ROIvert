#pragma once
#include <QDockWidget>
#include <QVBoxLayout>
#include <QSettings>

class DockWidgetWithSettings : public QDockWidget
{
    // Every dock widget is going to have a top level layout with no margins
    // Inside that is a toolbar with the buttons for reset and toggle storage
    //
    //
public:
    DockWidgetWithSettings(QWidget* parent = nullptr);
    virtual void saveSettings(QSettings& settings) const = 0;
    virtual void restoreSettings(QSettings& settings) = 0;
    virtual void resetSettings() = 0;

    QVBoxLayout toplay;
    bool getSettingsStorage() const;
    void setSettingsStorage(bool);
private:
    QAction* reset;
    QAction* save;
};
