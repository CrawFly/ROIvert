#include "widgets\TraceChartWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

#include "opencv2/opencv.hpp"

using ROIVert::NORMALIZATION;

namespace {
    void lerpGradAlpha(QLinearGradient& grad, QColor clr, float exp, float div) {
        for (int ii = 0; ii < 10; ++ii) {
            const float x = ii / 10.;
            clr.setAlphaF(pow(x, exp) / div);
            grad.setColorAt(x, clr);
        }
    }
    void cvMedIqr(cv::Mat mat, double& med, double& iqr) {
        // calculate median and iqr

        double valmin, valmax;
        cv::minMaxLoc(mat, &valmin, &valmax);

        if (valmin == valmax) {
            med = valmin;
            iqr = 1;
            return;
        }

        constexpr int n = 100;
        const float range[2] = { (float)valmin, (float)valmax };
        const float* histrange = { range };
        cv::Mat hist;
        cv::calcHist(&mat, 1, 0, cv::Mat(), hist, 1, &n, &histrange, true, false);

        hist /= mat.total();
        med = 0.;
        double q1 = 0., q3 = 0.;
        const double scale = (valmax - valmin);
        for (int i = 1; i <= n - 1; i++) {
            hist.at<float>(i) += hist.at<float>(i - 1);
            if (q1 == 0 && hist.at<float>(i) >= 0.25) { q1 = valmin + scale * (float)i / n; }
            if (med == 0 && hist.at<float>(i) >= 0.5) { med = valmin + scale * (float)i / n; }
            if (hist.at<float>(i) >= 0.75) { q3 = valmin + scale * (float)i / n; break; }
        }

        med = med;
        iqr = q3 - q1;
    }
}


struct TraceChartSeries::pimpl {
    void setData(cv::Mat data, NORMALIZATION norm);
    void updatePoly();
    void updateYExtents();
    void paint(QPainter& painter, const QColor& lineColor, const QColor& fillColor, const QTransform& T, const double& ymin);
    
    double extents[4] = { 0,1,0,1 };
    QString name;
    cv::Mat data;
    void setOffset(float) noexcept;
    float getOffset() const noexcept;
    QColor color{ Qt::red };

private:
    QPolygonF poly;
    QPainterPath path;
    float offset{ 0 };
};

TraceChartSeries::TraceChartSeries() = default; //?
TraceChartSeries::~TraceChartSeries() = default;
TraceChartSeries::TraceChartSeries(cv::Mat data, double xmin, double xmax, float offset, NORMALIZATION norm) {
    setXMin(xmin);
    setXMax(xmax);
    setData(data, norm);
    impl->setOffset(offset);
}
void TraceChartSeries::setData(cv::Mat data, float offset, NORMALIZATION norm) {
    impl->setData(data, norm);
    impl->setOffset(offset);
}
void TraceChartSeries::setData(cv::Mat data, NORMALIZATION norm) {
    impl->setData(data, norm);
}
void TraceChartSeries::paint(QPainter & painter, const QColor & lineColor, const QColor & fillColor, const QTransform & T, const double& ymin) {
    impl->paint(painter, lineColor, fillColor, T, ymin);
}

void TraceChartSeries::setXMin(const double& val) noexcept { impl->extents[0] = val; }
double TraceChartSeries::getXMin() const noexcept { return impl->extents[0]; }

void TraceChartSeries::setXMax(const double& val) noexcept { impl->extents[1] = val; }
double TraceChartSeries::getXMax() const noexcept { return impl->extents[1]; }

double TraceChartSeries::getYMin() const noexcept { return impl->extents[2]; }
double TraceChartSeries::getYMax() const noexcept { return impl->extents[3]; }

void TraceChartSeries::setOffset(float offset) noexcept { impl->setOffset(offset); }
float TraceChartSeries::getOffset() const noexcept { return impl->getOffset(); }

void TraceChartSeries::setColor(const QColor& color) noexcept { impl->color = color; }
QColor TraceChartSeries::getColor() const noexcept { return impl->color; };

QRectF TraceChartSeries::getExtents() {
    return QRectF(QPointF(impl->extents[0], impl->extents[2]), QPointF(impl->extents[1], impl->extents[3]));
}
    

// pimpl
void TraceChartSeries::pimpl::setData(cv::Mat d, NORMALIZATION norm) {
    if (d.depth() != 5) {
        auto fdata = cv::Mat(d);
        d.convertTo(fdata, CV_32F);
        data = fdata;
    }
    else {
        data = d;
    }
    switch (norm)
    {
    case ROIVert::NORMALIZATION::NONE:
        break;
    case ROIVert::NORMALIZATION::ZEROTOONE:
        cv::normalize(data, data, 1., 0., cv::NORM_MINMAX);
        break;
    case ROIVert::NORMALIZATION::L1NORM:
        cv::normalize(data, data, 1., 0., cv::NORM_L1);
        break;
    case ROIVert::NORMALIZATION::L2NORM:
        cv::normalize(data, data, 1., 0., cv::NORM_L2);
        break;
    case ROIVert::NORMALIZATION::ZSCORE:
        {
            cv::Scalar mu, sigma;
            cv::meanStdDev(data, mu, sigma);
            if (sigma == cv::Scalar::zeros()) {
                sigma = cv::Scalar(1, 1, 1, 1);
            }
            data = (data - mu) / sigma;
        }   
        break;
    case ROIVert::NORMALIZATION::MEDIQR:
        double med; double iqr;
        cvMedIqr(data, med, iqr);
        data = (data - med) / iqr;
        break;
    default:
        break;
    }
    updateYExtents();
    updatePoly();
}


void TraceChartSeries::pimpl::updatePoly() {
    poly.clear();
    if (data.empty()) return;

    const double mult = (extents[1] - extents[0]) / (data.size().width - 1);
    poly.reserve(data.size().width);

    for (int i = 0; i < data.size().width; ++i) {
        poly << QPointF(mult * i + extents[0], data.at<float>(0, i) + offset);
    }

    // Path used for filling
    QPolygonF p2(poly);
    p2.push_front(QPointF(extents[0], extents[2]));
    p2.push_back(QPointF(extents[1], extents[2]));
    path.clear();
    path.addPolygon(p2);
}
void TraceChartSeries::pimpl::updateYExtents() {
    if (data.empty()) {
        extents[2] = 0;
        extents[3] = 1;
    }
    else {
        cv::minMaxLoc(data, &extents[2], &extents[3]);
        extents[2] += offset;
        extents[3] += offset;
    }
    if (extents[2] == extents[3]) {
        extents[2] -= 1;
        extents[3] += 1;
    }
}

void TraceChartSeries::pimpl::setOffset(float off) noexcept {
    offset = off;
    updateYExtents();
    updatePoly();
}
float TraceChartSeries::pimpl::getOffset() const noexcept {
    return offset;
}


void TraceChartSeries::pimpl::paint(QPainter & painter, const QColor & lineColor, const QColor & fillColor, const QTransform & T, const double& ymin) {
    //todo: fill work
    /*
    if (style.Fill) {
        painter.setPen(Qt::NoPen);

        if (style.FillGradientToLineColor) {
            QLinearGradient grad(T.map(QPointF(extents[0], extents[2])),
                T.map(QPointF(extents[0], extents[3])));
            painter.setBrush(fillColor);

            QPointF orig_start = QPointF(path.elementAt(0));;
            QPointF orig_end = QPointF(path.elementAt(path.elementCount() - 1));

            path.setElementPositionAt(0, orig_start.x(), ymin);
            path.setElementPositionAt(path.elementCount() - 1, orig_end.x(), ymin);

            painter.drawPath(T.map(path).translated(QPointF(0, style.Width / 2)));

            path.setElementPositionAt(0, orig_start.x(), orig_start.y());
            path.setElementPositionAt(path.elementCount() - 1, orig_end.x(), orig_end.y());

            lerpGradAlpha(grad, lineColor, style.FillGradientExponent, style.FillGradientDivisor);
            painter.setBrush(grad);
        }
        else {
            painter.setBrush(fillColor);
        }
        painter.drawPath(T.map(path).translated(QPointF(0, style.Width / 2)));
    }
    */

    //todo: style work
    //painter.setPen(QPen(lineColor, style.Width));
    painter.setPen(QPen(color, 2.));
    painter.drawPolyline(T.map(poly));
}