#pragma once
#include <QObject>

class VideoControllerWidget;
class QPushButton;
class QSlider;
class QLabel;
class QLineEdit;
class QDial;

class tVideoControllerWidget : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void ttiming();
    void ttiming_data();
    
    void tdff();
    void tenabled();
    void tloop();
    void tsetnframes();
    void tspeeddial();
    void tspeeddial_data();
    void ttimelabel();
    void ttimelabel_data();
    void tforwback();
    void tframeoverflow();
    void tplaybutton();
private:
    VideoControllerWidget* widget;
    
    QPushButton *cmdBack = nullptr;
    QPushButton *cmdPlay = nullptr;
    QPushButton *cmdForw = nullptr;
    QPushButton *cmdLoop = nullptr;
    QSlider *sliScrub = nullptr;
    QLabel *lblTime = nullptr;
    QDial *dialSpeed = nullptr;
    QLineEdit *txtSpeed = nullptr;
    QPushButton *cmdDff = nullptr;
};

// some gui testing:
    // want to make sure the time label gets set correctly
    // want to make sure dff checker works, play button works, something about the wrapping? speed dial, speed selectorion...


/*
    bool isDff() const;
    void forceUpdate();

    void setNFrames(const size_t& frames);
    void setFrameRate(const float& framerate);
    
    size_t getCurrFrame() const noexcept;

    void play(const bool&);
    
    void start();
    void stop();
*/