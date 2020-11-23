#include "videocontroller.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QStyleOption>

VideoController::VideoController(QWidget *parent) : QWidget(parent)
{
    
    sliScrub->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    sliScrub->setMinimum(1);
    sliScrub->setSingleStep(1);
    sliScrub->setPageStep(1);

    cmdBack->setIcon(QIcon(":/icons/icons/vid_back.png"));
    cmdPlay->setIcon(QIcon(":/icons/icons/vid_play.png"));
    cmdForw->setIcon(QIcon(":/icons/icons/vid_forward.png"));
    cmdLoop->setIcon(QIcon(":/icons/icons/vid_repeat.png"));
    cmdPlay->setCheckable(true);
    cmdLoop->setCheckable(true);

    cmdBack->setToolTip(tr("Back one frame"));
    cmdPlay->setToolTip(tr("Play/Stop video"));
    cmdForw->setToolTip(tr("Forward one frame"));
    cmdLoop->setToolTip(tr("Toggle repeat mode"));

    lblTime->setText("00:00 (0)");
    QSize textSize = lblTime->fontMetrics().size(Qt::TextShowMnemonic, "00:00:000 (0000/0000)");
    lblTime->setMinimumSize(textSize);
    lblTime->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    dialSpeed->setWrapping(false);
    dialSpeed->setNotchesVisible(false);
    dialSpeed->setMinimum(-100);
    dialSpeed->setMaximum(100);
    dialSpeed->setValue(0);
    dialSpeed->setFixedWidth(60);
    dialSpeed->setToolTip(tr("Adjust playback speed"));

    QDoubleValidator* val = new QDoubleValidator;
    val->setBottom(.01);
    val->setTop(100);
    txtSpeed->setFixedWidth(40);
    txtSpeed->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    txtSpeed->setText("1");
    txtSpeed->setToolTip(tr("Set playback speed multiplier"));
    txtSpeed->setValidator(val);

    cmdDff->setText(QString::fromWCharArray(L"\x03B4\xD835\xDC53/\xD835\xDC53"));
    QSize ctextSize = cmdDff->fontMetrics().size(Qt::TextShowMnemonic, " " + cmdDff->text() + " ");
    cmdDff->setFixedWidth(ctextSize.width());
    cmdDff->setCheckable(true);
    

    QVBoxLayout *layTop = new QVBoxLayout;
    QGridLayout *layUnder = new QGridLayout;
    QHBoxLayout *layButtons = new QHBoxLayout;
    QHBoxLayout *layTxt = new QHBoxLayout;

    this->setLayout(layTop);
    layTop->addWidget(sliScrub);
    layTop->setSpacing(5);
    layTop->addLayout(layUnder);

    layUnder->addLayout(layButtons, 0, 1, Qt::AlignLeft);
    layButtons->addWidget(cmdBack);
    layButtons->addWidget(cmdPlay);
    layButtons->addWidget(cmdForw);
    layButtons->addWidget(cmdLoop);

    layButtons->addWidget(dialSpeed);
    layButtons->addWidget(txtSpeed);
    layButtons->addSpacing(15);
    layButtons->addWidget(cmdDff);
    layButtons->setSpacing(0);

    layUnder->addLayout(layTxt, 0, 2, Qt::AlignRight);
    layTxt->addWidget(lblTime);

    connect(cmdPlay, &QPushButton::clicked, this, &VideoController::PushPlay);
    connect(cmdBack, &QPushButton::clicked, this, [=]() { setFrame(currframe - 1); });
    connect(cmdForw, &QPushButton::clicked, this, [=]() { setFrame(currframe + 1); });
    connect(sliScrub, &QAbstractSlider::valueChanged, this, &VideoController::setFrame);
    connect(this->clock, &QTimer::timeout, this, &VideoController::clockStep);
    connect(dialSpeed, &QDial::valueChanged, this, &VideoController::setSpeedDial);
    connect(txtSpeed, &QLineEdit::editingFinished, this, &VideoController::setSpeedText);
    connect(cmdDff, &QPushButton::clicked, this, [=](bool checked) {emit frameChanged(currframe); emit dffToggle(checked); });

    clock->setTimerType(Qt::PreciseTimer);
    clock->setInterval(clockrate());
    lastframetime.start();

    setEnabled(false);
}

VideoController::~VideoController()
{
}

void VideoController::setFrame(const qint32 frame)
{
    if (frame != currframe && frame >= 1 && frame <= nframes())
    {
        currframe = frame;
        sliScrub->setValue(frame);
        updateTimeLabel();

        if (lastframetime.elapsed() >= 33)
        {
            emit frameChanged(frame);
            lastframetime.start();
        }
    }
}
void VideoController::setNFrames(const qint32 frames)
{
    setEnabled(frames > 0);
    sliScrub->setMaximum(frames);
    currframe = -1;
    setFrame(1);
    updateTimeLabel();
}
void VideoController::setStop()
{
    cmdPlay->setIcon(QIcon(":/icons/icons/vid_play.png"));
    cmdPlay->setChecked(false);
    clock->stop();
}

void VideoController::PushPlay(const bool &down)
{
    if (down)
    {
        cmdPlay->setIcon(QIcon(":/icons/icons/vid_stop.png"));
        timechecker.start();
        clock->setInterval(clockrate());
        clock->start();
    }
    else
    {
        cmdPlay->setIcon(QIcon(":/icons/icons/vid_play.png"));
        clock->stop();
    }
}
void VideoController::clockStep()
{
    //int ellapsed = lastframetime.elapsed(); // ms
    //int inc = floor((framerate * ellapsed * speedmult()) / 1000);
    int inc = 1;
    qint32 frame = currframe + inc;
    if (frame > nframes())
    {
        if (cmdLoop->isChecked())
        {
            frame = frame % nframes() + 1;
        }
        else
        {
            setStop();
            return;
        }
    }
    setFrame(frame);
}
void VideoController::setFrameRate(const float fr)
{
    framerate = fr;
    clock->setInterval(clockrate());
    updateTimeLabel();
}

void VideoController::updateTimeLabel()
{
    // qtime has to be specified as int, let's do it ms
    int ms = 1000 * (currframe - 1) / (framerate);
    QTime t(0, 0, 0, 0);
    t = t.addMSecs(ms);

    QString fmt = QString("ss:zzz (%1/%2)").arg(currframe).arg(nframes());
    if (t.minute() > 0)
    {
        fmt = QString("mm:ss:zzz (%1/%2)").arg(currframe).arg(nframes());
    }
    lblTime->setText(t.toString(fmt));
}
qint32 VideoController::nframes()
{
    return sliScrub->maximum();
}

void VideoController::setSpeedDial(const qint32 val)
{
    // val ranges from -100 to 100,
    float speed;

    if (val >= 0)
    {
        speed = round(pow(10, val / 50.));
    }
    else
    {
        speed = round(pow(10, val / 50.) * 100.) / 100.;
    }
    txtSpeed->setText(QString::number(speed));
    clock->setInterval(clockrate());
}
void VideoController::setSpeedText()
{
    float val = txtSpeed->text().toFloat();
    dialSpeed->setValue(log10(val) * 50);
    clock->setInterval(clockrate());
}

float VideoController::speedmult()
{
    return txtSpeed->text().toFloat();
}

int VideoController::clockrate()
{
    return 1000 / ((float)framerate * speedmult());
}