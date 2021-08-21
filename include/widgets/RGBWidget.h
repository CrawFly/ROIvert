#pragma once

#include <QWidget>
#include <QColor>

class RGBWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RGBWidget(QWidget *parent = nullptr);
    QColor getColor() const;

signals:
    void colorChanged(const QColor &);

public slots:
    void setColor(const QColor &);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};