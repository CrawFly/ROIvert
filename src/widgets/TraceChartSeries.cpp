#include "widgets/TraceChartWidget.h"

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include "opencv2/opencv.hpp"

#include "ChartStyle.h"

using ROIVert::NORMALIZATION;

namespace
{
    void lerpGradAlpha(QLinearGradient &grad, QColor clr)
    {
        const auto alp = clr.alphaF();
        for (int ii = 0; ii < 10; ++ii)
        {
            const float x = ii / 10.;
            clr.setAlphaF(x * alp);
            grad.setColorAt(x, clr);
        }
    }
    void cvMedIqr(cv::Mat mat, double &med, double &iqr)
    {
        // calculate median and iqr

        double valmin, valmax;
        cv::minMaxLoc(mat, &valmin, &valmax);

        if (valmin == valmax)
        {
            med = valmin;
            iqr = 1;
            return;
        }

        constexpr int n = 100;
        const float range[2] = {(float)valmin, (float)valmax};
        const float *histrange = {range};
        cv::Mat hist;
        cv::calcHist(&mat, 1, 0, cv::Mat(), hist, 1, &n, &histrange, true, false);

        hist /= mat.total();
        med = 0.;
        double q1 = 0., q3 = 0.;
        const double scale = (valmax - valmin);
        for (int i = 1; i <= n - 1; i++)
        {
            hist.at<float>(i) += hist.at<float>(i - 1);
            if (q1 == 0 && hist.at<float>(i) >= 0.25)
            {
                q1 = valmin + scale * (float)i / n;
            }
            if (med == 0 && hist.at<float>(i) >= 0.5)
            {
                med = valmin + scale * (float)i / n;
            }
            if (hist.at<float>(i) >= 0.75)
            {
                q3 = valmin + scale * (float)i / n;
                break;
            }
        }

        med = med;
        iqr = q3 - q1;
    }
}

struct TraceChartSeries::pimpl
{
    void setData(cv::Mat data);
    void updatePoly();
    void updateNorm();
    void updateYExtents();
    void paint(QPainter &painter, const QColor &lineColor, const QColor &fillColor, const QTransform &T, const double &ymin);

    double extents[4] = {0, 1, 0, 1};
    QString name;
    cv::Mat data;

    void setOffset(float) noexcept;
    float getOffset() const noexcept;
    std::shared_ptr<ChartStyle> chartstyle;

    bool polyContains(const QPointF &);
    bool highlighted{false};

    NORMALIZATION norm{NORMALIZATION::NONE}; // normalization used to calculate normdata...(cache)
    cv::Mat normdata;

private:
    QPolygonF poly;
    QPainterPath path;
    float offset{0};
};

TraceChartSeries::TraceChartSeries(std::shared_ptr<ChartStyle> style)
{
    impl->chartstyle = style;
}
TraceChartSeries::~TraceChartSeries() = default;

void TraceChartSeries::setStyle(std::shared_ptr<ChartStyle> style)
{
    impl->chartstyle = style;
}

void TraceChartSeries::setData(cv::Mat data)
{
    impl->setData(data);
}
void TraceChartSeries::paint(QPainter &painter, const QColor &lineColor, const QColor &fillColor, const QTransform &T, const double &ymin)
{
    impl->paint(painter, lineColor, fillColor, T, ymin);
}

void TraceChartSeries::setXMin(const double &val) noexcept { impl->extents[0] = val; }
double TraceChartSeries::getXMin() const noexcept { return impl->extents[0]; }

void TraceChartSeries::setXMax(const double &val) noexcept { impl->extents[1] = val; }
double TraceChartSeries::getXMax() const noexcept { return impl->extents[1]; }

double TraceChartSeries::getYMin() const noexcept { return impl->extents[2]; }
double TraceChartSeries::getYMax() const noexcept { return impl->extents[3]; }

void TraceChartSeries::setOffset(float offset) noexcept { impl->setOffset(offset); }
float TraceChartSeries::getOffset() const noexcept { return impl->getOffset(); }

void TraceChartSeries::setHighlighted(bool yesno) { impl->highlighted = yesno; }

QRectF TraceChartSeries::getExtents()
{
    return QRectF(QPointF(impl->extents[0], impl->extents[2]), QPointF(impl->extents[1], impl->extents[3]));
}

bool TraceChartSeries::polyContains(const QPointF &pt) { return impl->polyContains(pt); }

void TraceChartSeries::updatePoly()
{
    impl->updatePoly();
}

// pimpl
void TraceChartSeries::pimpl::setData(cv::Mat d)
{
    if (d.depth() != 5)
    {
        auto fdata = cv::Mat(d);
        d.convertTo(fdata, CV_32F);
        data = fdata;
    }
    else
    {
        data = d.clone();
    }

    normdata = data.clone();
    norm = NORMALIZATION::NONE;

    updateYExtents();
    updatePoly();
}

bool TraceChartSeries::pimpl::polyContains(const QPointF &pt) { return path.contains(pt); }

void TraceChartSeries::pimpl::updateNorm()
{

    switch (chartstyle->getNormalization())
    {
    case ROIVert::NORMALIZATION::NONE:
        normdata = data.clone();
        break;
    case ROIVert::NORMALIZATION::ZEROTOONE:
        cv::normalize(data, normdata, 1., 0., cv::NORM_MINMAX);
        break;
    case ROIVert::NORMALIZATION::L1NORM:
        cv::normalize(data, normdata, 1., 0., cv::NORM_L1);
        break;
    case ROIVert::NORMALIZATION::L2NORM:
        cv::normalize(data, normdata, 1., 0., cv::NORM_L2);
        break;
    case ROIVert::NORMALIZATION::ZSCORE:
    {
        cv::Scalar mu, sigma;
        cv::meanStdDev(data, mu, sigma);
        if (sigma == cv::Scalar::zeros())
        {
            sigma = cv::Scalar(1, 1, 1, 1);
        }
        normdata = (data.clone() - mu) / sigma;
    }
    break;
    case ROIVert::NORMALIZATION::MEDIQR:
        double med;
        double iqr;
        cvMedIqr(data, med, iqr);
        normdata = (data.clone() - med) / iqr;
        break;
    default:
        break;
    }
    updateYExtents();
}

void TraceChartSeries::pimpl::updatePoly()
{
    if (chartstyle->getNormalization() != norm)
    {
        updateNorm();
        norm = chartstyle->getNormalization();
    }

    poly.clear();
    if (normdata.empty())
        return;

    const double mult = (extents[1] - extents[0]) / (data.size().width - 1);
    poly.reserve(normdata.size().width);

    for (int i = 0; i < normdata.size().width; ++i)
    {
        poly << QPointF(mult * i + extents[0], normdata.at<float>(0, i) + offset);
    }

    // Path used for filling
    QPolygonF p2(poly);
    p2.push_front(QPointF(extents[0], extents[2]));
    p2.push_back(QPointF(extents[1], extents[2]));
    path.clear();
    path.addPolygon(p2);
}
void TraceChartSeries::pimpl::updateYExtents()
{
    if (normdata.empty())
    {
        extents[2] = 0;
        extents[3] = 1;
    }
    else
    {
        cv::minMaxLoc(normdata, &extents[2], &extents[3]);
        extents[2] += offset;
        extents[3] += offset;
    }
    if (extents[2] == extents[3])
    {
        extents[2] -= 1;
        extents[3] += 1;
    }
}

void TraceChartSeries::pimpl::setOffset(float off) noexcept
{
    offset = off;
    updateYExtents();
    updatePoly();
}
float TraceChartSeries::pimpl::getOffset() const noexcept
{
    return offset;
}

void TraceChartSeries::pimpl::paint(QPainter &painter, const QColor &lineColor, const QColor &fillColor, const QTransform &T, const double &ymin)
{
    if (chartstyle == nullptr)
    {
        return;
    }

    auto brush = chartstyle->getTraceBrush();
    auto pen = chartstyle->getTracePen();

    if (chartstyle->getTraceFillGradient() && brush.style() != Qt::NoBrush)
    {
        // A fill gradient
        QLinearGradient grad(T.map(QPointF(extents[0], extents[2])),
                             T.map(QPointF(extents[0], extents[3])));

        auto alp = brush.color().alphaF() > 0. ? 1 / brush.color().alphaF() : 1;
        lerpGradAlpha(grad, brush.color());
        brush = QBrush(grad);
    }

    if (highlighted)
    {
        if (pen.style() == Qt::NoPen && brush.style() == Qt::SolidPattern && brush.color().alphaF() < 1)
        {
            auto color = brush.color();
            color.setAlphaF(1.);
            brush.setColor(color);
        }
        else if (pen.style() == Qt::SolidLine)
        {
            const auto width = std::min(pen.width() * 2, pen.width() + 4);
            pen.setWidth(width);
        }
    }

    painter.setPen(Qt::NoPen);

    if (chartstyle->getDoBackBrush())
    {
        auto backbrush = QBrush(chartstyle->getBackgroundColor());
        painter.setBrush(backbrush);
        painter.drawPath(T.map(path).translated(QPointF(0, pen.width() / 2)));
    }

    painter.setBrush(brush);
    painter.drawPath(T.map(path).translated(QPointF(0, pen.width() / 2)));

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPolyline(T.map(poly));
}