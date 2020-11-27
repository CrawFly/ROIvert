#pragma once
// toolwindows is all of the various dockables:
//
//
//  1. Image Data:
//      .. done
//
//  2. Image Settings:
//       Histogram + min/max/alpha tool
//       raw | df/f
//       (for later) smooth?
//       (for later) colorizing options
//
//  3. ROI Settings:
//       Something to name them
//       Something for colors etc.
//       info about each one
//
//  4. (future) event detection...

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QLabel>
#include <QButtonGroup>
#include "contrastwidget.h"
#include "roivertcore.h"


namespace tool
{

    class imgData : public QWidget
    {
        Q_OBJECT

    public:
        imgData(QWidget *parent);
        ~imgData();

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
        void setContrast(float min, float max, float gamma);

    signals:
        void imgSettingsChanged(imgsettings newsettings);

    private:
        ContrastWidget* contrast;
        QButtonGroup* projection;
        const cv::ColormapTypes cmaps[5] = { cv::COLORMAP_DEEPGREEN , cv::COLORMAP_HOT , cv::COLORMAP_INFERNO, cv::COLORMAP_PINK, cv::COLORMAP_BONE};
        QComboBox* cmbColormap;
        QComboBox* cmbBlur;
        QSpinBox* spinBlurSize;
        QDoubleSpinBox* spinBlurSigma;
        QDoubleSpinBox* spinBlurSigmaI;
        QLabel* lblSigma;
        QLabel* lblSigmaI;
        QWidget* paramsWidg;
        
        void updateSettings();
    };

    class fileIO : public QWidget {
        Q_OBJECT

    public:
        fileIO(QWidget* parent);
        
    signals:
        void exportTraces(QString filename);
        void exportROIs(QString filename);
        void exportCharts(QString filename);
        // import ROIs
        // 
    private:
        QString cachepath;
    };
} // namespace tool
