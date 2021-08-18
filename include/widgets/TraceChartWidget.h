#pragma once
#include <QWidget>
#include "ROIVertEnums.h"

class TraceChartSeries;
class TraceChartAxis;
class ChartStyle;

namespace cv {
    class Mat;
}

class TraceChartWidget : public QWidget
{
    Q_OBJECT

public:
    TraceChartWidget(std::shared_ptr<ChartStyle> style = nullptr, QWidget* parent = nullptr);
    ~TraceChartWidget();
    void setStyle(std::shared_ptr<ChartStyle> style);

    void addSeries(std::shared_ptr<TraceChartSeries>);
    void removeSeries(std::shared_ptr<TraceChartSeries>) noexcept;
    std::vector<std::shared_ptr<TraceChartSeries>> getSeries() const;

    TraceChartAxis* getXAxis() const noexcept;
    TraceChartAxis* getYAxis() const noexcept;

    void setTitle(const QString& title) noexcept;
    QString getTitle() const noexcept;

    void setAntiAliasing(bool) noexcept;
    bool getAntiAliasing() const noexcept;

    void saveAsImage(const QString& filename, int outputwidth = -1, int outputheight = -1, int quality = -1);
    QSize minimumSizeHint() const override;

    void updateExtents();


signals:
    void chartClicked(TraceChartWidget*, std::vector<TraceChartSeries*>, Qt::KeyboardModifiers);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

class TraceChartSeries
{
public:
    TraceChartSeries(std::shared_ptr<ChartStyle> = nullptr);
    ~TraceChartSeries();
    void setStyle(std::shared_ptr<ChartStyle> style);

    void setData(cv::Mat, float offset, ROIVert::NORMALIZATION norm = ROIVert::NORMALIZATION::NONE);
    void setData(cv::Mat, ROIVert::NORMALIZATION norm = ROIVert::NORMALIZATION::NONE);
    cv::Mat getData() const noexcept;

    void setXMin(const double&) noexcept;
    double getXMin() const noexcept;
    
    void setXMax(const double&) noexcept;
    double getXMax() const noexcept;

    double getYMin() const noexcept;
    double getYMax() const noexcept;
    
    QRectF getExtents();
    
    void paint(QPainter& painter, const QColor& lineColor, const QColor& fillColor, const QTransform& T, const double& ymin);

    void setOffset(float) noexcept;
    float getOffset() const noexcept;
    
    void setStyle(const ChartStyle&) noexcept;

    bool polyContains(const QPointF&);
    void setHighlighted(bool); // todo: highlighted means double thickness lines? What if there are no lines?
    
private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

class TraceChartAxis
{
public:
    TraceChartAxis(std::shared_ptr<ChartStyle> style = nullptr);
    virtual ~TraceChartAxis();
    virtual void paint(QPainter& painter) = 0;
    void setStyle(std::shared_ptr<ChartStyle> style);
    
    void setExtents(const double& min, const double& max);
    std::tuple<double, double> getExtents() const noexcept;
    std::tuple<double, double> getLimits() const;

    void setLabel(const QString& Label);
    virtual QString getLabel() const noexcept;

    virtual void setLength(const int& length) = 0;  // Length corresponds to the direction of the axle * note that this doesn't affect layout!
    virtual int getLength() const noexcept = 0;

    virtual std::tuple<double, double> getMargins() const = 0;   // The length corresponds to the length of the axle, margins include any text that overhangs
    virtual int getThickness() const noexcept = 0;               // Thickness is in the perpendecular direction to the axis. 
    
    void setZero(const int& xzero, const int& yzero) noexcept;
    
    void setSpacings(const int& label, const int& ticklabel, const int& tickmark) noexcept;
    void setTickLength(const int& ticklength) noexcept;
    
    void setMaxNTicks(const unsigned int& n);

    void setVisible(bool) noexcept;
    bool getVisible() const noexcept;

protected:
    // this updates the tick values, the ticklabelthickness, and the margins. It needs to be called if the font changes or the labels change.
    virtual void updateLayout();
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

class TraceChartHAxis : public TraceChartAxis {
public:
    TraceChartHAxis(std::shared_ptr<ChartStyle> style = nullptr);
    void paint(QPainter& painter) override;
    void setLength(const int& length) noexcept override;
    int getLength() const noexcept override;
    std::tuple<double, double> getMargins() const override;
    int getThickness() const noexcept override;

protected:
    void updateLayout() override;
};

class TraceChartVAxis : public TraceChartAxis {
public:
    TraceChartVAxis(std::shared_ptr<ChartStyle> style = nullptr);
    void paint(QPainter& painter) override;
    void setLength(const int& length) noexcept override;
    int getLength() const noexcept override;
    std::tuple<double, double> getMargins() const override;
    int getThickness() const noexcept override;

protected:
    void updateLayout() override;

};


class RidgeLineWidget : public TraceChartWidget
{
public:
    RidgeLineWidget(std::shared_ptr<ChartStyle> = nullptr, QWidget* parent = nullptr);
    void updateOffsets();
};

