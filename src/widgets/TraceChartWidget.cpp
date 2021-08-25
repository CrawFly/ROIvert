#include "widgets\TraceChartWidget.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

#include "ChartStyle.h"

struct TraceChartWidget::pimpl {
    std::vector<std::shared_ptr<TraceChartSeries>> series;
    std::unique_ptr<TraceChartHAxis> xaxis = std::make_unique<TraceChartHAxis>(chartstyle);
    std::unique_ptr<TraceChartVAxis> yaxis = std::make_unique<TraceChartVAxis>(chartstyle);

    QString titlestring;
    bool antialias;
    QFontMetrics titlefontmea = QFontMetrics(QFont());


    QSize estimateMinimumSize();
    void updateExtents();
    void paint(QPainter& painter, const QRect& contentsRect);
    void setInnerMargins(const QMargins& marg) noexcept;

    std::shared_ptr<ChartStyle> chartstyle;
    QPointF pos2data(QPoint pos) {
        QPointF hitpos;
        if (plotbox.contains(pos)) {
            // convert to data units:
            double xmin, xmax, ymin, ymax;
            std::tie(xmin, xmax) = xaxis->getLimits();
            std::tie(ymin, ymax) = yaxis->getLimits();

            hitpos.setX(xmin + (xmax - xmin) * (pos.x() - plotbox.left()) / static_cast<float>(plotbox.width()));
            hitpos.setY(ymin + (ymax - ymin) * (plotbox.bottom() - pos.y()) / static_cast<float>(plotbox.height()));
        }
        return hitpos;
    }
    std::vector<TraceChartSeries*> getHitSeries(QPointF datapos) {
        std::vector<TraceChartSeries*> ret;
        if (!datapos.isNull()) {
            for (auto &s:series) {
                if (s->polyContains(datapos)) {
                    ret.push_back(s.get());
                }
            }
        }
        return ret;
        
    }


private:
    QMargins margins;
    QRect plotbox;
    int titlethickness = 0;
    int titlespace = 8;
    int axlegapv, axlegaph;

    void calcPlotbox();
    void paint_background(QPainter& painter);
    void paint_title(QPainter& painter);
    void paint_axes(QPainter& painter, const int& left);
    void paint_data(QPainter& painter);
};

TraceChartWidget::TraceChartWidget(std::shared_ptr<ChartStyle> style, QWidget* parent)
    : QWidget(parent)
{
    if (style) {
        setStyle(style);
    }   

    setContentsMargins(0, 0, 0, 0); // these margins are in the layout
    impl->setInnerMargins(QMargins(5, 5, 5, 5));
    
}

TraceChartWidget::~TraceChartWidget() = default;

void TraceChartWidget::setStyle(std::shared_ptr<ChartStyle> style) {
    impl->chartstyle = style;
    updateStyle();
}
ChartStyle* TraceChartWidget::getStyle() {
    return impl->chartstyle.get();
}
void TraceChartWidget::updateStyle() {
    impl->xaxis->setStyle(impl->chartstyle);
    impl->yaxis->setStyle(impl->chartstyle);

    for (auto& s : impl->series) {
        s->updatePoly();
    }
    // set y axis label
    auto norm = impl->chartstyle->getNormalization();
    switch (norm)
    {
    case ROIVert::NORMALIZATION::NONE:
        impl->yaxis->setLabel("df/f");
        break;
    case ROIVert::NORMALIZATION::ZEROTOONE:
        impl->yaxis->setLabel("df/f (0-1)");
        break;
    case ROIVert::NORMALIZATION::L1NORM:
        impl->yaxis->setLabel("df/f (L1 Norm)");
        break;
    case ROIVert::NORMALIZATION::L2NORM:
        impl->yaxis->setLabel("df/f (L2 Norm)");
        break;
    case ROIVert::NORMALIZATION::ZSCORE:
        impl->yaxis->setLabel("df/f (z units)");
        break;
    case ROIVert::NORMALIZATION::MEDIQR:
        impl->yaxis->setLabel("df/f (IQR units)");
        break;
    default:
        break;
    }

    updateExtents();
    update();
}

void TraceChartWidget::addSeries(std::shared_ptr<TraceChartSeries> series) {
    impl->series.push_back(series);
    impl->updateExtents();
}
void TraceChartWidget::removeSeries(std::shared_ptr<TraceChartSeries> series) noexcept {
    auto it = std::find(impl->series.begin(), impl->series.end(), series);
    if (it != impl->series.end()) {
        impl->series.erase(it);
    }
    impl->updateExtents();
}
std::vector<std::shared_ptr<TraceChartSeries>> TraceChartWidget::getSeries() const {
    return impl->series;
}
TraceChartAxis* TraceChartWidget::getXAxis() const noexcept {
    return impl->xaxis.get();
}
TraceChartAxis* TraceChartWidget::getYAxis() const noexcept {
    return impl->yaxis.get();
}
void TraceChartWidget::setTitle(const QString& title) noexcept {
    impl->titlestring = title;
}
QString TraceChartWidget::getTitle() const noexcept {
    return impl->titlestring;
}
void TraceChartWidget::setAntiAliasing(bool onoff) noexcept {
    impl->antialias = onoff;
}
bool TraceChartWidget::getAntiAliasing() const noexcept {
    return impl->antialias;
}
void TraceChartWidget::saveAsImage(const QString & filename, int w, int h, int quality) {
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

QSize TraceChartWidget::minimumSizeHint() const {
    return impl->estimateMinimumSize();
}
void TraceChartWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    impl->paint(painter, contentsRect());
}
void TraceChartWidget::updateExtents() {
    impl->updateExtents();
}

// pimpl
QSize TraceChartWidget::pimpl::estimateMinimumSize() {
    const int h = 2 * (titlefontmea.height() + xaxis->getThickness());
    const int w = 2 * yaxis->getThickness();
    return QSize(w, h);
}
void TraceChartWidget::pimpl::updateExtents() {
    QRectF r;
    if (series.empty()) {
        xaxis->setExtents(0, 1);
        yaxis->setExtents(0, 1);
    }

    for (auto& s : series) {
        if (s) {
            r |= s->getExtents();
        }
    }
    xaxis->setExtents(r.left(), r.right());
    yaxis->setExtents(r.top(), r.bottom());

}
void TraceChartWidget::pimpl::paint(QPainter& painter, const QRect& contentsRect) {
    
    if (antialias) {
        painter.setRenderHint(QPainter::Antialiasing);
    }

    plotbox = contentsRect;
    paint_background(painter);

    // Adjust n-ticks based on overall size, poor man's estimate
    xaxis->setMaxNTicks(std::max(contentsRect.width() / 80, 5));
    yaxis->setMaxNTicks(std::max(contentsRect.height() / 80, 5));

    calcPlotbox();
    paint_title(painter);

    // Draw Axes
    paint_axes(painter, contentsRect.left() + margins.left());

    // Draw children
    painter.setClipRect(plotbox + QMargins(0, chartstyle->getAxisPen().width(), 0, chartstyle->getAxisPen().width()));
    paint_data(painter);
}
void TraceChartWidget::pimpl::calcPlotbox() {
    // subtract inner margins
    plotbox -= margins;

    titlethickness = 0;
    if (!titlestring.isEmpty()) {
        titlethickness = titlefontmea.height() + titlespace;
    }
    plotbox -= QMargins(0, titlethickness, 0, 0);
    

    const int xthickness{ xaxis->getVisible() ? xaxis->getThickness() : 0 };
    const int ythickness{ yaxis->getVisible() ? yaxis->getThickness() : 0 };
    const int xleftmarg{ (int)std::get<0>(xaxis->getMargins()) };
    const int xrightmarg{ xaxis->getVisible() ? (int)std::get<1>(xaxis->getMargins()) : 0 };
    const int ymarg{ yaxis->getVisible() ? (int)std::get<0>(yaxis->getMargins()) : 0 }; // top and bottom are the same
    plotbox -= QMargins(std::max(ythickness, xleftmarg), ymarg, xrightmarg, xthickness);
    
    auto lw{ chartstyle->getAxisPen().widthF() };
    axlegaph = xaxis->getVisible() ? std::clamp(plotbox.width() * .03, 2. + lw, 40.) : 5; 
    axlegapv = yaxis->getVisible() ? std::clamp(plotbox.height() * .04, 2. + lw, 40.) : 5;
    plotbox -= QMargins(axlegaph, 0, 0, axlegapv);

}
void TraceChartWidget::pimpl::paint_background(QPainter & painter) {
    if (chartstyle == nullptr) {
        return;
    }
    painter.setPen(Qt::NoPen); 
    painter.setBrush(chartstyle->getBackgroundColor());
    painter.drawRect(plotbox);
}
void TraceChartWidget::pimpl::paint_title(QPainter & painter) {
    const QRect titler(plotbox.left(),
        plotbox.top() - titlethickness - titlespace,
        plotbox.width(),
        titlethickness - titlespace);
    
    auto font = QFont();
    painter.setFont(QFont());
    painter.setPen(Qt::black);

    painter.drawText(titler, titlestring, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
}
void TraceChartWidget::pimpl::paint_axes(QPainter & painter, const int& left) {
        
    if (xaxis->getVisible()) {
        xaxis->setZero(plotbox.left(), plotbox.bottom() + axlegapv);
        // todo: this is moot now that i'm just passing plotbox?
        xaxis->setLength(plotbox.width()); 
        xaxis->setPlotBox(plotbox);
        xaxis->paint(painter);
    }

    if (yaxis->getVisible()) {
        yaxis->setZero(left, plotbox.top());
        yaxis->setLength(plotbox.height());
        yaxis->setPlotBox(plotbox);
        yaxis->paint(painter);
    }
}
void TraceChartWidget::pimpl::paint_data(QPainter & painter) {
    double xmin, xmax, ymin, ymax;
    std::tie(xmin, xmax) = xaxis->getLimits();
    std::tie(ymin, ymax) = yaxis->getLimits();

    QTransform T;
    T = QTransform::fromTranslate(-xmin, -ymin) *
        QTransform::fromScale(plotbox.width() / (xmax - xmin), -plotbox.height() / (ymax - ymin)) *
        QTransform::fromTranslate(plotbox.left(), static_cast<double>(plotbox.top()) + static_cast<double>(plotbox.height()));

    for (int i = 0; i < series.size(); ++i) {
        const QColor lineColor(Qt::red);
        const QColor fillColor(Qt::red);

        //const QColor lineColor(style.Line.Colors[i % style.Line.Colors.size()]);
        //const QColor fillColor = QColor(style.Line.FillColors[i % style.Line.FillColors.size()]);
        series[i]->paint(painter, lineColor, fillColor, T, ymin);
    }
}
void TraceChartWidget::pimpl::setInnerMargins(const QMargins& marg) noexcept {
    margins = marg;
}

void TraceChartWidget::mousePressEvent(QMouseEvent* event) {
    // convert hit position to data position:
    QPoint pos = event->pos();

    if (!contentsRect().contains(pos)) {
        // this probably can't be hit in the current state
        emit chartClicked(nullptr, std::vector<TraceChartSeries*>(), event->modifiers());
    }

    auto hitTraces = impl->getHitSeries(impl->pos2data(pos));
    
    emit chartClicked(this, hitTraces, event->modifiers());
}
