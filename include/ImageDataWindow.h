#pragma once
#include <memory>
#include "DialogWithSettings.h"

class ImageDataWindow : public DialogWithSettings
{
    Q_OBJECT

public:
    explicit ImageDataWindow(QWidget *parent = nullptr);
    ~ImageDataWindow();
    void saveSettings(QSettings &settings) const override;
    void restoreSettings(QSettings &settings) override;
    void resetSettings() override;

signals:
    void fileLoadRequested(std::vector<std::pair<QString, size_t>> filenameframelist, const double frameRate, const int dsTime, const int dsSpace);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
