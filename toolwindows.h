#pragma once
// toolwindows is all of the various dockables:

// todo: refactor and pimplify
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QLabel>
#include "roivertcore.h"

#include "widgets/ContrastWidget.h"
#include "widgets/ColorPickWidget.h"
#include "widgets/ProjectionPickWidget.h"
#include "widgets/ColormapPickWidget.h"
#include "widgets/SmoothingPickWidget.h"

namespace tool
{

    class imgData : public QWidget
    {
        Q_OBJECT

    public:
        imgData(QWidget *parent);

    signals:
        void fileLoadRequested(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace);
        void frameRateChanged(double fr);


    public slots:
        void fileLoadCompleted(size_t nframes, size_t height, size_t width);
        void setProgBar(int val);

    private:
        QLineEdit *txtFilePath = new QLineEdit(this);
        QPushButton *cmdBrowseFilePath = new QPushButton(this);
        QDoubleSpinBox *spinFrameRate = new QDoubleSpinBox(this);
        QSpinBox *spinDownTime = new QSpinBox(this);
        QSpinBox *spinDownSpace = new QSpinBox(this);
        QLabel *lblFileInfo = new QLabel(this);
        QProgressBar* progBar = new QProgressBar(this);
        QPushButton *cmdLoad = new QPushButton(this);

        void load();

    private slots:
        void browse();
        void filePathChanged(const QString &filepath);
    };

    class imgSettings : public QWidget {
        Q_OBJECT

    public:
        imgSettings(QWidget* parent);
        ~imgSettings();
        void setHistogram(std::vector<float> &data);
        void setContrast(ROIVert::contrast c);

    signals:
        void imgSettingsChanged(ROIVert::imgsettings newsettings);

    private:
        ContrastWidget* contrast;
        ProjectionPickWidget* projection;
        ColormapPickWidget* colormap;
        SmoothingPickWidget* Wsmoothing;
        
        void updateSettings();
    };

    class fileIO : public QWidget {
        Q_OBJECT

    public:
        fileIO(QWidget* parent);
        
    signals:
        void exportTraces(QString filename, bool doHeader, bool doTimeCol);
        void exportROIs(QString filename);
        void importROIs(QString filename);
        void exportCharts(QString filename, int width, int height, int quality, bool ridge);
        
    private:
        QString cachepath; //TODO: replace with windowFilePath
    };

} // namespace tool
