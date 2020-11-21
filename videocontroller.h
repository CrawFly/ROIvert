#pragma once

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <QIntValidator>
#include <QTimer>
#include <QTime>
#include <QDial>
#include <QCheckBox>

class VideoController : public QWidget
{
    Q_OBJECT

public:
    VideoController(QWidget *parent);
    ~VideoController();
    bool dff() { return cmdDff->isChecked(); }
    void forceUpdate() { qint32 f = currframe; currframe = 0; setFrame(f); };
signals:
    void frameChanged(const qint32 &frame);
    void dffToggle(bool isdff);

public slots:
    void setFrame(const qint32 frame);
    void setNFrames(const qint32 frames);
    void setFrameRate(const float framerate);
    void setStop(); // Forces a video to stop, no emit.

private:
    QPushButton *cmdBack = new QPushButton(this);
    QPushButton *cmdPlay = new QPushButton(this);
    QPushButton *cmdForw = new QPushButton(this);
    QPushButton *cmdLoop = new QPushButton(this);
    QSlider *sliScrub = new QSlider(Qt::Horizontal, this);
    QLabel *lblTime = new QLabel(this);
    QTimer *clock = new QTimer(this);
    QDial *dialSpeed = new QDial(this);
    QLineEdit *txtSpeed = new QLineEdit(this);
    QPushButton* cmdDff = new QPushButton(this);

    qint32 currframe = 0;
    qint32 framerate = 30;
    QTime lastframetime;

    void PushPlay(const bool &flag);
    void clockStep();
    void updateTimeLabel(); //
    qint32 nframes();
    float speedmult();
    void setSpeedDial(const qint32 val);
    void setSpeedText();

    int clockrate();

    QTime timechecker;
};
