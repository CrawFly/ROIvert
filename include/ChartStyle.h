#pragma once
#include <QObject>
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
    
    void setLabelFontSize(int);
    void setTickLabelFontSize(int);
    void setFontFamily(QString);
    QFont getLabelFont();
    QFont getTickLabelFont();
    QFontMetrics getLabelFontMetrics();
    QFontMetrics getTickLabelFontMetrics();



    QPen getAxisPen() const;
    bool getGrid() const noexcept;

    
    // Traces
    void setTraceLineWidth(int);
    void setTraceFillOpacity(int);
    void setTraceFillGradient(bool);
    void setDoBackBrush(bool);
    bool getDoBackBrush() const noexcept;
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

