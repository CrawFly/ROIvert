#pragma once
#include <QDockWidget>

class ImageDataWidget : public QDockWidget
{       
Q_OBJECT

public:
    ImageDataWidget(QWidget *parent = nullptr);
    void setContentsEnabled(bool);
    signals:
        void fileLoadRequested(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace);
        void frameRateChanged(double fr);

    public slots:
        void setProgBar(int val);

    private:
        struct pimpl;
        std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

