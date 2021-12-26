#pragma once
#include <QObject>
#include "ROI/ROIStyle.h"
#include "ROIVertEnums.h"

class ChartStyle : public QObject {
    Q_OBJECT
public:
    ChartStyle();
    ChartStyle& operator=(const ChartStyle&) noexcept;
    ChartStyle(const ChartStyle&);
    ~ChartStyle();

    void connectToROIStyle(const ROIStyle*);

    // Charts
    void setBackgroundColor(QColor) noexcept;
    QColor getBackgroundColor() const noexcept;

    // Axes
    void setAxisColor(QColor) noexcept;
    void setAxisLineWidth(int) noexcept;
    void setGrid(bool) noexcept;

    void setLabelFontSize(int) noexcept;
    void setTickLabelFontSize(int) noexcept;
    void setFontFamily(QString) noexcept;
    QFont getLabelFont();
    QFont getTickLabelFont();
    QFontMetrics getLabelFontMetrics();
    QFontMetrics getTickLabelFontMetrics();
    void setLimitStyle(ROIVert::LIMITSTYLE) noexcept;
    ROIVert::LIMITSTYLE getLimitStyle() const noexcept;

    QPen getAxisPen() const;
    bool getGrid() const noexcept;

    // Traces
    void setTraceLineWidth(int) noexcept;
    void setTraceFillOpacity(int) noexcept;
    void setTraceFillGradient(bool) noexcept;
    void setDoBackBrush(bool) noexcept;
    bool getDoBackBrush() const noexcept;
    QPen getTracePen() const;
    QBrush getTraceBrush() const;
    void setNormalization(ROIVert::NORMALIZATION) noexcept;
    ROIVert::NORMALIZATION getNormalization() const noexcept;
    bool getTraceFillGradient() const noexcept;

signals:
    void ColorChange();

public slots:
    void ROIStyleChanged(const ROIStyle&);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
