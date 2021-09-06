#include "widgets/VideoControllerWidget.h"

#include <QBoxLayout>
#include <QDial>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QTime>
#include <QTimer>
#include <QValidator>
#include <QDebug>

struct VideoControllerWidget::pimpl
{

    bool isDff() const;
    void initWidgets();
    void layoutWidgets(VideoControllerWidget *par);

    bool setFrame(const size_t &);
    void setSpeed_dial(const int &);
    void setSpeed_text();

    float speedmult();
    int clockrate();

    size_t nframes() const;
    void updateTimeLabel();

    size_t currframe = 0;
    float framerate = 30.;

    QPushButton *cmdBack = new QPushButton;
    QPushButton *cmdPlay = new QPushButton;
    QPushButton *cmdForw = new QPushButton;
    QPushButton *cmdLoop = new QPushButton;
    QSlider *sliScrub = new QSlider(Qt::Horizontal);
    QLabel *lblTime = new QLabel();
    QDial *dialSpeed = new QDial();
    QLineEdit *txtSpeed = new QLineEdit();
    QPushButton *cmdDff = new QPushButton();

    QTimer timer;
    QTime elapsed;
    int accumtime{0};
};

VideoControllerWidget::VideoControllerWidget(QWidget *parent) : QWidget(parent)
{
    impl->initWidgets();
    impl->layoutWidgets(this);

    connect(impl->cmdPlay, &QPushButton::clicked, this, &VideoControllerWidget::play);
    connect(impl->cmdBack, &QPushButton::clicked, this, &VideoControllerWidget::decFrame);
    connect(impl->cmdForw, &QPushButton::clicked, this, &VideoControllerWidget::incFrame);
    connect(impl->sliScrub, &QAbstractSlider::valueChanged, this, &VideoControllerWidget::setFrame);
    connect(impl->dialSpeed, &QDial::valueChanged, this, [=](int speed)
            { impl->setSpeed_dial(speed); });
    connect(impl->txtSpeed, &QLineEdit::editingFinished, this, [=]
            { impl->setSpeed_text(); });
    connect(impl->cmdDff, &QPushButton::clicked, this, &VideoControllerWidget::toggleDff);

    connect(&impl->timer, &QTimer::timeout, this, &VideoControllerWidget::timestep);
}

VideoControllerWidget::~VideoControllerWidget() = default;

void VideoControllerWidget::forceUpdate()
{
    impl->sliScrub->setValue(impl->currframe);
    impl->updateTimeLabel();
    emit frameChanged(impl->currframe);
}
void VideoControllerWidget::setFrame(const size_t &frame)
{
    if (impl->setFrame(frame))
    {
        emit frameChanged(frame);
    }
}
void VideoControllerWidget::setNFrames(const size_t &frames)
{
    setEnabled(frames > 0);
    impl->sliScrub->setMaximum(frames);
    impl->currframe = 1;
    forceUpdate();
}
void VideoControllerWidget::setFrameRate(const float &framerate)
{
    impl->framerate = framerate;
    impl->updateTimeLabel();
}
void VideoControllerWidget::stop()
{
    impl->timer.stop();
    impl->cmdPlay->setChecked(false);
}
void VideoControllerWidget::start()
{
    impl->timer.start();
    impl->elapsed.restart();
}
bool VideoControllerWidget::isDff() const { return impl->isDff(); }
void VideoControllerWidget::toggleDff(bool checked) const
{
    emit frameChanged(getCurrFrame());
    emit dffToggled(checked);
}

void VideoControllerWidget::dffToggle(const bool &isdff)
{
    impl->cmdDff->setChecked(isdff);
    toggleDff(isdff);
}

void VideoControllerWidget::decFrame() { setFrame(getCurrFrame() - 1); }
void VideoControllerWidget::incFrame() { setFrame(getCurrFrame() + 1); }
size_t VideoControllerWidget::getCurrFrame() const noexcept { return impl->currframe; }
void VideoControllerWidget::play(const bool &pressed)
{
    if (pressed)
    {
        start();
    }
    else
    {
        stop();
    }
}
void VideoControllerWidget::timestep()
{
    impl->accumtime += impl->elapsed.restart();
    const size_t nframes_adv = impl->accumtime / impl->clockrate();
    impl->accumtime -= nframes_adv * impl->clockrate();
    size_t frame = (impl->currframe + nframes_adv);
    const auto nframes = impl->nframes();

    if (frame > nframes)
    {
        if (impl->cmdLoop->isChecked())
        {
            frame = frame % nframes + 1;
        }
        else if (impl->currframe < nframes)
        {
            frame = nframes;
        }
        else
        {
            stop();
            return;
        }
    }
    setFrame(frame); // note that this will be a no-op if the frame hasn't changed
}

// **** impl **** //
bool VideoControllerWidget::pimpl::isDff() const { return cmdDff->isChecked(); };

void VideoControllerWidget::pimpl::initWidgets()
{
    sliScrub->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    sliScrub->setMinimum(1);
    sliScrub->setMaximum(1);
    sliScrub->setSingleStep(1);
    sliScrub->setPageStep(1);

    cmdBack->setIcon(QIcon(":/icons/vid_back.png"));
    cmdPlay->setIcon(QIcon(":/icons/vid_play.png"));
    cmdForw->setIcon(QIcon(":/icons/vid_forward.png"));
    cmdLoop->setIcon(QIcon(":/icons/vid_repeat.png"));
    cmdPlay->setCheckable(true);
    cmdLoop->setCheckable(true);

    cmdBack->setToolTip(tr("Back one frame"));
    cmdPlay->setToolTip(tr("Play/Stop video"));
    cmdForw->setToolTip(tr("Forward one frame"));
    cmdLoop->setToolTip(tr("Toggle repeat mode"));

    lblTime->setText("00:00 (0)");
    const QSize textSize = lblTime->fontMetrics().size(Qt::TextShowMnemonic, "00:00:000 (0000/0000)");
    lblTime->setMinimumSize(textSize);
    lblTime->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    dialSpeed->setWrapping(false);
    dialSpeed->setNotchesVisible(false);
    dialSpeed->setMinimum(-100);
    dialSpeed->setMaximum(100);
    dialSpeed->setValue(0);
    dialSpeed->setFixedWidth(60);
    dialSpeed->setToolTip(tr("Adjust playback speed"));

    auto val = std::make_unique<QDoubleValidator>();
    val->setBottom(.01);
    val->setTop(100);
    txtSpeed->setFixedWidth(40);
    txtSpeed->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    txtSpeed->setText("1");
    txtSpeed->setToolTip(tr("Set playback speed multiplier"));
    txtSpeed->setValidator(val.get());

    cmdDff->setText("df/f");
    cmdDff->setCheckable(true);

    timer.setInterval(16);
}

void VideoControllerWidget::pimpl::layoutWidgets(VideoControllerWidget *par)
{
    QVBoxLayout *layTop = new QVBoxLayout;
    QGridLayout *layUnder = new QGridLayout;
    QHBoxLayout *layButtons = new QHBoxLayout;
    QHBoxLayout *layTxt = new QHBoxLayout;

    par->setLayout(layTop);
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
}

bool VideoControllerWidget::pimpl::setFrame(const size_t &frame)
{
    if (frame == currframe || frame < 1 || frame > nframes())
    {
        return false;
    }
    currframe = frame;
    sliScrub->setValue(frame);
    updateTimeLabel();
    return true;
}
void VideoControllerWidget::pimpl::setSpeed_dial(const int &val)
{
    // val ranges from -100 to 100
    const float speed = val >= 0 ? round(pow(10, val / 50.)) : round(pow(10, val / 50.) * 100.) / 100.;
    txtSpeed->setText(QString::number(speed));
}
void VideoControllerWidget::pimpl::setSpeed_text()
{
    float val = txtSpeed->text().toFloat();
    dialSpeed->setValue(log10(val) * 50);
}

size_t VideoControllerWidget::pimpl::nframes() const
{
    return sliScrub->maximum();
}

void VideoControllerWidget::pimpl::updateTimeLabel()
{
    const size_t fr = currframe == 0 ? 0 : currframe - 1;

    const int ms = 1000 * fr / (framerate);
    QTime t(0, 0, 0, 0);
    t = t.addMSecs(ms);

    QString fmt = QString("ss:zzz (%1/%2)").arg(currframe).arg(nframes());
    if (t.minute() > 0)
    {
        fmt = QString("mm:ss:zzz (%1/%2)").arg(currframe).arg(nframes());
    }
    lblTime->setText(t.toString(fmt));
}

float VideoControllerWidget::pimpl::speedmult() { return txtSpeed->text().toFloat(); }
int VideoControllerWidget::pimpl::clockrate() { return std::max(1.f, 1000 / (framerate * speedmult())); }