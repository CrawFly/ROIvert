#pragma once
#include <QWidget>
class VideoControllerWidget : public QWidget
{
    Q_OBJECT

public:
    VideoControllerWidget(QWidget *parent);
    bool isDff() const;
    void toggleDff(bool checked) const; // todo: move to private (not impl)
    void forceUpdate();

    void setFrame(const size_t& frame); // todo: move to private (not impl)
    void decFrame();                    // todo: move to private (not impl)
    void incFrame();                    // todo: move to private (not impl)
    void setNFrames(const size_t& frames);
    void setFrameRate(const float& framerate);
    
    size_t getCurrFrame() const noexcept;

    void play(const bool&);
    
    void start();
    void stop();
    
    void timestep();                    // todo: move to private (not impl)
signals:
    void frameChanged(const size_t& frame) const;
    void dffToggle(const bool& isdff) const;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

