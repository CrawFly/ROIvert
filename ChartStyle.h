#pragma once
#include "QObject"
#include "ROI/ROIStyle.h"

class ChartStyle : public QObject {
    Q_OBJECT
public:
    ChartStyle();
    ChartStyle& operator=(const ChartStyle&);
    ChartStyle(const ChartStyle&);
    ~ChartStyle();
    
    void connectToROIStyle(ROIStyle*);

    // Charts
    void setBackgroundColor(QColor);
    QColor getBackgroundColor() const noexcept;

    // Axes
    void setAxisColor(QColor);
    void setAxisLineWidth(int);
    void setGrid(bool);
    void setTitleFontSize(int); //todo: getters for fontsizes
    void setLabelFontSize(int);
    void setTickLabelFontSize(int);
    QPen getAxisPen() const;
    bool getGrid() const noexcept;

    
    // Traces
    void setTraceLineWidth(int);
    void setTraceFillOpacity(int);
    void setTraceFillGradient(bool);
    QPen getTracePen() const;
    QBrush getTraceBrush() const;

signals:
    void StyleChanged(const ChartStyle&);

public slots:
    void ROIStyleChanged(const ROIStyle&);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

