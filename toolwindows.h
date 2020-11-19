#pragma once
// toolwindows is all of the various dockables:
//
//
//  1. Image Data:
//       LineEdit File Path + Browse button
//       (future) File List (checkable?)
//       (future) Filter/ordering tools
//       (future) drop target
//       LineEdit Frame Rate
//       Downsample Time chk + spinner
//       Downsample Space chk + spinner
//       Label - file info
//       Push - Load Commit
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

#include <QValidator>
#include <QLabel>

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

    public slots:
        void fileLoadCompleted(size_t nframes, size_t height, size_t width);

    private:
        QLineEdit *txtFilePath = new QLineEdit(this);
        QPushButton *cmdBrowseFilePath = new QPushButton(this);
        QDoubleSpinBox *spinFrameRate = new QDoubleSpinBox(this);
        QSpinBox *spinDownTime = new QSpinBox(this);
        QSpinBox *spinDownSpace = new QSpinBox(this);
        QLabel *lblFileInfo = new QLabel(this);
        QPushButton *cmdLoad = new QPushButton(this);

        void load();

    private slots:
        void browse();
        void filePathChanged(const QString &filepath);
    };
} // namespace tool

//  1. Image Data:
//       Label - file info
//       Push - Load Commit
//