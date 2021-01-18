#pragma once

#include <memory>

class QString;
class QPolygonF;
class QPainterPath;
class QRectF;
class QPointF;


class QPainter;
class QColor;
class QTransform;


namespace cv {
    class Mat;
}

namespace TraceChart {
    enum class NORM;
    struct LineStyle;
}

class Series
{
public:
    Series(cv::Mat data, double xmin, double xmax, QString name, double offset, TraceChart::NORM norm);
    ~Series();

    QString getName() noexcept;
    void setName(QString name) noexcept;

    double getXMin() noexcept;
    void setXMin(const double& val) noexcept;

    double getXMax() noexcept;
    void setXMax(const double& val) noexcept;

    double getYMin() noexcept;
    double getYMax() noexcept;

    QRectF getExtents();

    void paint(QPainter& painter, const QColor& lineColor, const QColor& fillColor, const QTransform& T, const double& ymin);

    void setData(cv::Mat data, double offset, TraceChart::NORM norm);

    void setStyle(const TraceChart::LineStyle& s);
    TraceChart::LineStyle getStyle();
    
    bool polyContains(QPointF pt);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};