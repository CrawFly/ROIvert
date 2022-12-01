#pragma once
#include <memory>
#include <cmath>
#include <QObject>

class QGraphicsView;

class ZoomPan : public QObject
{
    Q_OBJECT

public:
    ZoomPan(QGraphicsView* view);
    ~ZoomPan();

    void setZoomFactor(double value) noexcept;
    void setModifier(Qt::KeyboardModifier);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
    bool eventFilter(QObject* object, QEvent* event) override;
};
