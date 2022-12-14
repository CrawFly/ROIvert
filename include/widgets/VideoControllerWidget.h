#pragma once
#include <QWidget>
class VideoControllerWidget : public QWidget
{
    Q_OBJECT

public:
    VideoControllerWidget(QWidget* parent = nullptr);
    ~VideoControllerWidget();

    bool isDff() const;
    void forceUpdate();

    void setNFrames(const size_t& frames);
    void setFrameRate(const float& framerate);

    size_t getCurrFrame() const noexcept;

    void play(const bool&);

    void start();
    void stop();

signals:
    void frameChanged(const size_t& frame) const;
    void dffToggled(const bool& isdff) const;

public slots:
    void dffToggle(const bool& isdff);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
    void setFrame(const size_t& frame);
    void decFrame();
    void incFrame();
    void toggleDff(bool checked) const;
    void timestep();
};
