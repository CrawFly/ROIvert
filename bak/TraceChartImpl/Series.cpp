#include "widgets/TraceChartImpl/Series.h"
#include "widgets/TraceChartImpl/Core.h"

#include <QString>
#include <QPolygon>
#include <QPainterPath>
#include <QRect>
#include <QDebug>
#include <QPainter>

#include "opencv2/opencv.hpp"

using namespace TraceChart;

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
struct Series::pimpl {
    void updatePoly();
    void updateYExtents();
    void paint(QPainter& painter, const QColor& lineColor, const QColor& fillColor, const QTransform& T, const double& ymin);
    void setStyle(const LineStyle& s) { style = s; }
    LineStyle getStyle() { return style; };

    double extents[4] = { 0,1,0,1 };
    QString name;
    cv::Mat data;
    bool polyContains(QPointF pt);

private:
    QPolygonF poly;
    QPainterPath path;
    LineStyle style;
};

Series::Series(cv::Mat data, double xmin = 0, double xmax = 1, QString name = "", double offset = 0., NORM norm = NORM::NONE) {
    impl->extents[0] = xmin;
    impl->extents[1] = xmax;
    impl->name = name;

    setData(data, offset, norm);
}

Series::~Series() = default;

QString Series::getName() noexcept { return impl->name; }
void Series::setName(QString name) noexcept { impl->name = name; }

double Series::getXMin() noexcept { return impl->extents[0]; }
void Series::setXMin(const double& val) noexcept { impl->extents[0] = val; }

double Series::getXMax() noexcept { return impl->extents[1]; }
void Series::setXMax(const double& val) noexcept { impl->extents[1] = val; }

double Series::getYMin() noexcept { return impl->extents[2]; }
double Series::getYMax() noexcept { return impl->extents[3]; }

QRectF Series::getExtents() {
    return QRectF(QPointF(impl->extents[0], impl->extents[2]), QPointF(impl->extents[1], impl->extents[3]));
}

void Series::setData(cv::Mat data, double offset, NORM norm) {
    
    if (data.depth() != 5) {
        auto fdata = cv::Mat(data);
        data.convertTo(fdata, CV_32F);
        impl->data = fdata;
    }
    else {
        impl->data = data;
    }

    switch (norm)
    {
    case NORM::ZEROTOONE:
        cv::normalize(impl->data, impl->data, 1., 0., cv::NORM_MINMAX);
        break;
    case NORM::L1NORM:
        cv::normalize(impl->data, impl->data, 1., 0., cv::NORM_L1);
        break;
    case NORM::L2NORM:
        cv::normalize(impl->data, impl->data, 1., 0., cv::NORM_L2);
        break;
    case NORM::ZSCORE:
    {
        cv::Scalar mu, sigma;
        cv::meanStdDev(impl->data, mu, sigma);
        if (sigma == cv::Scalar::zeros()) {
            sigma = cv::Scalar(1, 1, 1, 1);
        }

        impl->data = (impl->data - mu) / sigma;
    }
    break;
    case NORM::MEDIQR:
        double med; double iqr;
        cvMedIqr(impl->data, med, iqr);
        impl->data = (impl->data - med) / iqr;
        break;
    }

    impl->data += offset;

    impl->updateYExtents();
    impl->updatePoly();
}

void Series::setStyle(const LineStyle & s) { impl->setStyle(s); }
LineStyle Series::getStyle() { return impl->getStyle(); };


void Series::paint(QPainter & painter, const QColor & lineColor, const QColor & fillColor, const QTransform & T, const double& ymin) {
    impl->paint(painter, lineColor, fillColor, T, ymin);
}

bool Series::polyContains(QPointF pt) {
    return impl->polyContains(pt);
}

void Series::pimpl::updatePoly() {
    poly.clear();
    if (data.empty()) return;

    const double mult = (extents[1] - extents[0]) / (data.size().width - 1);
    poly.reserve(data.size().width);
    for (int i = 0; i < data.size().width; ++i) {
        poly << QPointF(mult * i + extents[0], data.at<float>(0, i));
    }

    // Path used for filling
    QPolygonF p2(poly);
    p2.push_front(QPointF(extents[0], extents[2]));
    p2.push_back(QPointF(extents[1], extents[2]));
    path.clear();
    path.addPolygon(p2);
}
void Series::pimpl::updateYExtents() {
    if (data.empty()) {
        extents[2] = 0;
        extents[3] = 1;
    }
    else {
        cv::minMaxLoc(data, &extents[2], &extents[3]);
    }
    if (extents[2] == extents[3]) {
        extents[2] -= 1;
        extents[3] += 1;
    }
}
void Series::pimpl::paint(QPainter & painter, const QColor & lineColor, const QColor & fillColor, const QTransform & T, const double& ymin) {
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

    painter.setPen(QPen(lineColor, style.Width));
    painter.drawPolyline(T.map(poly));
}

bool Series::pimpl::polyContains(QPointF pt) {
    return path.contains(pt);
}