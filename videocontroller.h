#pragma once

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QDial>
#include <QCheckBox>


class VideoController : public QWidget
{
    Q_OBJECT

public:
    VideoController(QWidget *parent);
    const bool dff();
    void forceUpdate();
    void setNFrames(const size_t frames);
    void setFrameRate(const float framerate);
    float getFrameRate() const;
    void setStop(); // Forces a video to stop, no emit.

signals:
    void frameChanged(const size_t &frame);
    void dffToggle(bool isdff);
        
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

    size_t currframe = 0;
    float framerate = 30.;
    QTime lastframetime;

    void setFrame(const size_t frame);
    void PushPlay(const bool &flag);
    void clockStep();
    void updateTimeLabel(); 
    void setSpeedDial(const int val);
    void setSpeedText();

    const size_t nframes();
    const float speedmult();
    const int clockrate();
};
