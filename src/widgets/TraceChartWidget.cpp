#include "widgets\TraceChartWidget.h"
#include "widgets\TraceChartImpl\Axis.h"
#include "widgets\TraceChartImpl\Series.h"

#include <QPainter>
#include <QPainterPath>
#include <QSize>
#include <QMouseEvent>

#include "opencv2/opencv.hpp"
#include "assert.h"

#include <QDebug>

using namespace TraceChart;


struct TraceChartWidget::pimpl {
    pimpl() {
        setStyle(style);
    }

    QString titlestring;
    HAxis xaxis;
    VAxis yaxis;

    void setData(cv::Mat, QStringList, const std::vector<double>&, const std::vector<double>&, const std::vector<double>&, NORM norm);
    void removeData(const QStringList&);
    void clearData();
    void updateExtents();
    void paint(QPainter& painter, const QRect& contentsRect);
    QSize estimateMinimumSize();

    void setStyle(ChartStyle);
    ChartStyle getStyle();
    void setInnerMargins(const QMargins& marg) noexcept { margins = marg; };
    QMargins getInnerMargins() noexcept { return margins; };

    bool AntiAlias = false;

    QPointF pos2data(QPoint pos) {
        QPointF hitpos;
        
        if (plotbox.contains(pos)) {
            // convert to data units:
            double xmin, xmax, ymin, ymax;
            std::tie(xmin, xmax) = xaxis.getLimits();
            std::tie(ymin, ymax) = yaxis.getLimits();

            hitpos.setX(xmin + (xmax - xmin) * (pos.x() - plotbox.left()) / (float)plotbox.width());
            hitpos.setY(ymin + (ymax - ymin) * (plotbox.bottom() - pos.y()) / (float)plotbox.height());
        }

        return hitpos;
    }

    int getHitObj(QPointF datapos) {
        int ind = -1;
        if (!datapos.isNull()) {
            // We'll call the hit object the first (last?) one (?) in series that's hit?
            for (int i = 0; i < series.size(); i++) {
                if (series[i]->polyContains(datapos)) {
                    ind = i;
                }
            }
        }
        return ind;
    }
        

private:
    int getSeriesIndex(const QString& name);
    void calcPlotbox();

    void paint_background(QPainter& painter);
    void paint_title(QPainter& painter);
    void paint_axes(QPainter& painter, const int& left);
    void paint_data(QPainter& painter);

    int titlespace = 8;
    double xminmax[2] = { 0,1 };
    double yminmax[2] = { 0,1 };
    std::vector<std::unique_ptr<Series>> series; // like to do this with unique_ptr...but i might have made a mess with getseries?
    QFontMetrics titlefontmea = QFontMetrics(QFont());
    ChartStyle style;
    QRect plotbox;
    QMargins margins;
    int titlethickness = 0;
    int axlegapv, axlegaph;
};

TraceChartWidget::TraceChartWidget(QWidget* parent)
    : QWidget(parent)
{
    // default contents margins?
    setContentsMargins(11, 11, 11, 11);
    setInnerMargins(QMargins(5, 5, 5, 5));
}
TraceChartWidget::~TraceChartWidget() = default;


void TraceChartWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    impl->paint(painter, contentsRect());
}

void TraceChartWidget::setData(cv::Mat datas, QStringList names, const std::vector<double>&xmins, const std::vector<double>&xmaxs, const std::vector<double>&offsets, NORM norm) {
    assert(datas.size().height == names.size()); // todo: exception!
    impl->setData(datas, names, xmins, xmaxs, offsets, norm);
    update();
}
void TraceChartWidget::setData(cv::Mat data, QString name, const double& xmin, const double& xmax, const double& offset, NORM norm) {
    assert(data.size().height == 1); // todo: to replace this with a exception

    const std::vector<double> xmins = { xmin };
    const std::vector<double> xmaxs = { xmax };
    const std::vector<double> offsets = { offset };

    setData(data, QStringList(name), xmins, xmaxs, offsets, norm);
}
void TraceChartWidget::removeData(const QString & name) {
    removeData(QStringList(name));
}
void TraceChartWidget::removeData(const QStringList & names) {
    impl->removeData(names);
    update();
}
void TraceChartWidget::clearData() {
    impl->clearData();
    update();
}
void TraceChartWidget::setStyle(ChartStyle style) {
    impl->setStyle(style);
}
ChartStyle TraceChartWidget::getStyle() {
    return impl->getStyle();
}
QSize TraceChartWidget::minimumSizeHint() const {
    return impl->estimateMinimumSize();
}
void TraceChartWidget::setInnerMargins(QMargins marg) {
    impl->setInnerMargins(marg);
}
QMargins TraceChartWidget::getInnerMargins() {
    return impl->getInnerMargins();
}

void TraceChartWidget::setTitle(const QString & title) noexcept { impl->titlestring = title; }
void TraceChartWidget::setXLabel(const QString & xlabel) { impl->xaxis.setLabel(xlabel); }
void TraceChartWidget::setYLabel(const QString & ylabel) { impl->yaxis.setLabel(ylabel); }
QString TraceChartWidget::getTitle() const noexcept { return impl->titlestring; }
QString TraceChartWidget::getXLabel() const noexcept { return impl->xaxis.getLabel(); }
QString TraceChartWidget::getYLabel() const noexcept { return impl->yaxis.getLabel(); }

void TraceChartWidget::saveAsImage(const QString & filename, int w, int h, int quality) {
    // cache some properties to set back after:
    const auto oldmarg = contentsMargins();

    const bool doresize = w > 0 || h > 0;
    const auto oldmax = maximumSize();
    const auto oldmin = minimumSize();

    if (doresize) {
        const auto minsize = impl->estimateMinimumSize();
        setFixedSize(std::max(minsize.width(), w), std::max(minsize.height(), h));
        // *** consider adding a flag for normalzie fonts...that might be a tad tricky :)
    }
    setContentsMargins(0, 0, 0, 0);
    update();
    QPixmap pm(size());
    render(&pm, QPoint(), QRegion(), QWidget::DrawChildren);
    pm.save(filename, nullptr, quality);

    setContentsMargins(oldmarg);
    if (doresize) {
        setMinimumSize(oldmin);
        setMaximumSize(oldmax);
    }
}

void TraceChartWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MouseButton::LeftButton) {
        QPointF hitpos = impl->pos2data(event->pos());
        int hitobj = impl->getHitObj(hitpos);
        emit clicked(hitpos, hitobj);
    }
    
    QWidget::mousePressEvent(event);

}

/* ----- PIMPL ------ */
void TraceChartWidget::pimpl::setData(cv::Mat datas, QStringList names, const std::vector<double>&xmins, const std::vector<double>&xmaxs, const std::vector<double> &offsets, NORM norm) {
    for (size_t i = 0; i < names.size(); i++) {
        auto row = datas.row(i);
        auto ind = getSeriesIndex(names[i]);

        if (ind == -1) {
            series.push_back(std::make_unique<Series>(datas, xmins[i], xmaxs[i], names[i], offsets[i], norm));
            series.back()->setStyle(style.Line);
        }
        else {
            series[ind]->setXMin(xmins[i]);
            series[ind]->setXMax(xmaxs[i]);
            series[ind]->setData(row, offsets[i], norm);
        }
    }
    updateExtents();
}
void TraceChartWidget::pimpl::removeData(const QStringList & names) {
    for (auto& name : names) {
        const auto ind = getSeriesIndex(name);
        series.erase(series.begin() + ind);
    }
    updateExtents();
}
void TraceChartWidget::pimpl::clearData() {
    series.clear();
    updateExtents();
}
void TraceChartWidget::pimpl::updateExtents() {
    QRectF r;
    if (series.empty()) {
        xaxis.setExtents(0, 1);
        yaxis.setExtents(0, 1);
    }

    for (auto& s : series) {
        if (s) {
            r |= s->getExtents();
        }
    }
    xaxis.setExtents(r.left(), r.right());
    yaxis.setExtents(r.top(), r.bottom());
}
int TraceChartWidget::pimpl::getSeriesIndex(const QString & name) {
    for (int i = 0; i < series.size(); ++i) {
        if (series[i] && series[i]->getName() == name) {
            return i;
        }
    }
    return -1;
}
QSize TraceChartWidget::pimpl::estimateMinimumSize() {
    // Minimum height is:
    //  1.2*(titlefontheight (*istitle) + xaxis.labelfontheight(*islabel) + ticklabelfontheight) + 20?
    const int h = 2 * (titlefontmea.height() + xaxis.getThickness());
    const int w = 2 * yaxis.getThickness();
    return QSize(w, h);
}
void TraceChartWidget::pimpl::paint(QPainter & painter, const QRect & cRect) {
    QSize foo;


    if (AntiAlias) {
        painter.setRenderHint(QPainter::Antialiasing);
    }

    plotbox = cRect;
    paint_background(painter);

    // Adjust n-ticks based on overall size, poor man's estimate
    xaxis.setMaxNTicks(std::max(cRect.width() / 80, 5));
    yaxis.setMaxNTicks(std::max(cRect.height() / 80, 5));

    calcPlotbox();
    paint_title(painter);

    // Draw Axes
    paint_axes(painter, cRect.left() + margins.left());

    // Draw children
    painter.setClipRect(plotbox + QMargins(0, style.Line.Width, 0, style.Line.Width));
    paint_data(painter);
}

void TraceChartWidget::pimpl::setStyle(ChartStyle newstyle) {
    style = newstyle;
    titlefontmea = QFontMetrics(style.Title.Font);
    xaxis.setStyle(style.Axis);
    yaxis.setStyle(style.Axis);
    for (auto& s : series) {
        s->setStyle(style.Line);
    }

}
ChartStyle TraceChartWidget::pimpl::getStyle() {
    return style;
}

void TraceChartWidget::pimpl::calcPlotbox() {
    // subtract inner margins
    plotbox -= margins;

    titlethickness = 0;
    if (!titlestring.isEmpty()) {
        titlethickness = titlefontmea.height() + titlespace;
    }
    plotbox -= QMargins(0, titlethickness, 0, 0);

    if (style.Axis.Show) {
        // Get thickness of each axis, will determine things below
        const int xthickness = style.Axis.ShowX ? xaxis.getThickness() : 0;
        const int ythickness = style.Axis.ShowY ? yaxis.getThickness() : 0;
        // need to use max with margs...or we end up not quite getting it right when we shut of y axis (for e.g.)

        const int xleftmarg = std::get<0>(xaxis.getMargins());
        const int xrightmarg = style.Axis.ShowX ? std::get<1>(xaxis.getMargins()) : 0;
        const int ymarg = style.Axis.ShowY ? std::get<0>(yaxis.getMargins()) : 0; // top and bottom are the same

        plotbox -= QMargins(std::max(ythickness, xleftmarg), ymarg, xrightmarg, xthickness);

        axlegaph = style.Axis.ShowX ? std::clamp(plotbox.width() * .03, 2. + style.Line.Width, 40.) : 5;
        axlegapv = style.Axis.ShowY ? std::clamp(plotbox.height() * .04, 2. + style.Line.Width, 40.) : 5;
        plotbox -= QMargins(axlegaph, 0, 0, axlegapv);
    }
}
void TraceChartWidget::setAntiAliasing(bool onoff) noexcept { impl->AntiAlias = onoff; }
bool TraceChartWidget::getAntiAliasing() const noexcept { return impl->AntiAlias; }

void TraceChartWidget::pimpl::paint_background(QPainter & painter) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(style.Background.Color);
    painter.drawRect(plotbox);
}
void TraceChartWidget::pimpl::paint_title(QPainter & painter) {
    const QRect titler(plotbox.left(),
        plotbox.top() - titlethickness - titlespace,
        plotbox.width(),
        titlethickness - titlespace);
    painter.setFont(style.Title.Font);
    painter.setPen(style.Title.Color);
    painter.drawText(titler, titlestring, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
}

void TraceChartWidget::pimpl::paint_axes(QPainter & painter, const int& left) {
    if (style.Axis.ShowX) {
        xaxis.setZero(plotbox.left(), plotbox.bottom() + axlegapv);
        xaxis.setLength(plotbox.width());
        xaxis.paint(painter);
    }

    if (style.Axis.ShowY) {
        yaxis.setZero(left, plotbox.top());
        yaxis.setLength(plotbox.height());
        yaxis.paint(painter);
    }
}
void TraceChartWidget::pimpl::paint_data(QPainter & painter) {
    double xmin, xmax, ymin, ymax;
    std::tie(xmin, xmax) = xaxis.getLimits();
    std::tie(ymin, ymax) = yaxis.getLimits();

    QTransform T;
    T = QTransform::fromTranslate(-xmin, -ymin) *
        QTransform::fromScale(plotbox.width() / (xmax - xmin), -plotbox.height() / (ymax - ymin)) *
        QTransform::fromTranslate(plotbox.left(), static_cast<double>(plotbox.top()) + static_cast<double>(plotbox.height()));

    for (int i = 0; i < series.size(); ++i) {
        const QColor lineColor(style.Line.Colors[i % style.Line.Colors.size()]);
        const QColor fillColor = QColor(style.Line.FillColors[i % style.Line.FillColors.size()]);
        series[i]->paint(painter, lineColor, fillColor, T, ymin);
    }
}
