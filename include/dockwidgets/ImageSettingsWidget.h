#pragma once
#include <QDockWidget>
#include "roivertcore.h"

class ImageSettingsWidget : public QDockWidget
{
    Q_OBJECT
    public:
        ImageSettingsWidget(QWidget* parent = nullptr);
        ~ImageSettingsWidget();
        void setHistogram(std::vector<float> &data);
        void setContrast(ROIVert::contrast c);
        void setContentsEnabled(bool);

    signals:
        void imgSettingsChanged(ROIVert::imgsettings newsettings);
        void dffToggled(bool);
    
    public slots:
        void dffToggle(bool);

    private:
        struct pimpl;
        std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

