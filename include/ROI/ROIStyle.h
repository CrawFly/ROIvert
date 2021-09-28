#pragma once
#include <QObject>
#include <QColor>
#include <QPen>
#include <QBrush>


class ROIStyle : public QObject
{
    Q_OBJECT
public:
    ROIStyle();
    ROIStyle& operator=(const ROIStyle&);
    ROIStyle(const ROIStyle&);

    ~ROIStyle();

    QPen getPen() const;
    QBrush getBrush() const;

    void setColor(QColor color);
    void setLineColor(QColor color);
    QColor getLineColor() const noexcept;
    void setFillColor(QColor color);
    QColor getFillColor() const noexcept;
    void setSelectedColor(QColor color);
    void setUnselectedColor(QColor color);

    void setLineWidth(int linewidth);
    void setFillOpacity(int opacity);

    void setSelectorSize(int size);
    void setColorBySelected(bool cbs);

    int getSelectorSize() const noexcept;
    bool isColorBySelected() const noexcept;
    
signals:
    void StyleChanged(const ROIStyle&);

public slots:
    void setSelected(bool);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};

class ROIPalette : QObject
{
    Q_OBJECT
public:
    void setPaletteColors(std::vector<QColor>);
    std::vector<QColor> getPaletteColors() const noexcept;
    std::vector<QColor> getPaletteColors(std::vector<size_t> inds) const;
    QColor getPaletteColor(size_t ind) const noexcept;

signals:
    void paletteChanged();

private:
    std::vector<QColor> palettecolors = { QColor("#FFBE00"),QColor("#0055C8"), QColor("#82C8E6"),QColor("#A0006E"),
                                      QColor("#FF5A00"),QColor("#82DC73"),QColor("#FF82B4"),QColor("#D2D200"),QColor("#D282BE"),
                                      QColor("#DC9600"),QColor("#6E491E"),QColor("#00643C"),QColor("#82C8E6"),QColor("#640082"),
                                      QColor("#A81032"),QColor("#96C047"),QColor("#E170A7"),QColor("#147A88"),QColor("#6E6E00") };
};
