#include "widgets\TraceChartWidget.h"
#include <QPainter>
#include <QDebug>

struct TraceChartWidget::pimpl {
    std::vector<std::shared_ptr<TraceChartSeries>> series;
    std::unique_ptr<TraceChartHAxis> xaxis = std::make_unique<TraceChartHAxis>();
    std::unique_ptr<TraceChartVAxis> yaxis = std::make_unique<TraceChartVAxis>();

    QString titlestring;
    bool antialias;
    QFontMetrics titlefontmea = QFontMetrics(QFont());


    QSize estimateMinimumSize();
    void updateExtents();
    void paint(QPainter& painter, const QRect& contentsRect);
    void setInnerMargins(const QMargins& marg) noexcept;

    //todo: need pos2data when impl mouse stuff


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

TraceChartWidget::TraceChartWidget(QWidget* parent)
    : QWidget(parent)
{
    //setContentsMargins(11, 11, 11, 11);
    setContentsMargins(5, 5, 5, 5);
    impl->setInnerMargins(QMargins(5, 5, 5, 5));
    
}

TraceChartWidget::~TraceChartWidget() = default;

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
    // todo: with style, temp linewidth = 2
    //painter.setClipRect(plotbox + QMargins(0, style.Line.Width, 0, style.Line.Width));
    painter.setClipRect(plotbox + QMargins(0, 2, 0, 2));
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


    // todo: style work
    /*
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
    */
    const int xthickness{ xaxis->getThickness() };
    const int ythickness{ yaxis->getThickness() };
    const int xleftmarg = std::get<0>(xaxis->getMargins());
    const int xrightmarg = std::get<1>(xaxis->getMargins());
    const int ymarg = std::get<0>(yaxis->getMargins()); // top and bottom are the same
    plotbox -= QMargins(std::max(ythickness, xleftmarg), ymarg, xrightmarg, xthickness);
    axlegaph = std::clamp(plotbox.width() * .03, 2. + 2., 40.); //todo: note forced linewidth when moving to style...
    axlegapv = std::clamp(plotbox.height() * .04, 2. + 2., 40.);
    plotbox -= QMargins(axlegaph, 0, 0, axlegapv);
}
void TraceChartWidget::pimpl::paint_background(QPainter & painter) {
    painter.setPen(Qt::NoPen);
    //todo: style work
    //painter.setBrush(style.Background.Color);
    painter.setBrush(Qt::white);
    painter.drawRect(plotbox);
}
void TraceChartWidget::pimpl::paint_title(QPainter & painter) {
    const QRect titler(plotbox.left(),
        plotbox.top() - titlethickness - titlespace,
        plotbox.width(),
        titlethickness - titlespace);

    //todo: style work
    //painter.setFont(style.Title.Font);
    //painter.setPen(style.Title.Color);
    auto font = QFont();
    painter.setFont(QFont());
    painter.setPen(Qt::black);

    painter.drawText(titler, titlestring, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
}
void TraceChartWidget::pimpl::paint_axes(QPainter & painter, const int& left) {
    // todo: style work
    //if (style.Axis.ShowX) {
        xaxis->setZero(plotbox.left(), plotbox.bottom() + axlegapv);
        xaxis->setLength(plotbox.width());
        xaxis->paint(painter);
    //}

    //if (style.Axis.ShowY) {
        yaxis->setZero(left, plotbox.top());
        yaxis->setLength(plotbox.height());
        yaxis->paint(painter);
    //}
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