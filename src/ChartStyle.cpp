#include "ChartStyle.h"

#include <QFont>
#include <QFontMetrics>
#include <QDebug>

struct ChartStyle::pimpl
{
    QColor backgroundcolor{ Qt::white };
    QColor axiscolor{ Qt::black };
    int axislinewidth{ 1 };
    bool grid = false;
    int labelfontsize{ 12 };
    int tickfontsize{ 10 };
    QString fontfamily{ "Arial" };

    int tracelinewidth{ 2 };
    int tracefillopacity{ 55 };
    bool tracefillgradient{ true };
    QColor linecolor{ Qt::red };
    QColor fillcolor{ Qt::red };

    bool dobackbrush{ false };
    ROIVert::NORMALIZATION normalization{ ROIVert::NORMALIZATION::NONE };

    ROIVert::LIMITSTYLE limitstyle{ ROIVert::LIMITSTYLE::AUTO };
};

ChartStyle::ChartStyle() : impl(std::make_unique<pimpl>()) { };

ChartStyle::~ChartStyle() {}

ChartStyle& ChartStyle::operator=(const ChartStyle& that) noexcept
{
    if (this != &that)
    {
        impl = std::make_unique<pimpl>();
        *(impl) = *(that.impl);
    }
    return *this;
}

ChartStyle::ChartStyle(const ChartStyle& that) : impl(std::make_unique<pimpl>())
{
    if (that.impl != nullptr && this != &that)
    {
        *(impl) = *(that.impl);
    }
}

// Charts
void ChartStyle::setBackgroundColor(QColor c) noexcept
{
    impl->backgroundcolor = c;
}
QColor ChartStyle::getBackgroundColor() const noexcept
{
    return impl->backgroundcolor;
}

// Axes
void ChartStyle::setAxisColor(QColor c) noexcept
{
    impl->axiscolor = c;
}
void ChartStyle::setAxisLineWidth(int w) noexcept
{
    impl->axislinewidth = w;
}
void ChartStyle::setGrid(bool onoff) noexcept
{
    impl->grid = onoff;
}
void ChartStyle::setLabelFontSize(int fs) noexcept
{
    impl->labelfontsize = fs;
}
void ChartStyle::setTickLabelFontSize(int fs) noexcept
{
    impl->tickfontsize = fs;
}

void ChartStyle::setFontFamily(QString font) noexcept
{
    impl->fontfamily = font;
}
QFont ChartStyle::getLabelFont()
{
    QFont font;
    font.setFamily(impl->fontfamily);
    font.setPointSize(impl->labelfontsize);
    return font;
}
QFont ChartStyle::getTickLabelFont()
{
    QFont font;
    font.setFamily(impl->fontfamily);
    font.setPointSize(impl->tickfontsize);
    return font;
}
QFontMetrics ChartStyle::getLabelFontMetrics()
{
    return QFontMetrics(getLabelFont());
}
QFontMetrics ChartStyle::getTickLabelFontMetrics()
{
    return QFontMetrics(getTickLabelFont());
}

QPen ChartStyle::getAxisPen() const
{
    return QPen(impl->axiscolor, impl->axislinewidth);
}
bool ChartStyle::getGrid() const noexcept
{
    return impl->grid;
}
// Traces
void ChartStyle::setTraceLineWidth(int w) noexcept
{
    impl->tracelinewidth = w;
}
void ChartStyle::setTraceFillOpacity(int o) noexcept
{
    impl->tracefillopacity = o;
}
void ChartStyle::setTraceFillGradient(bool onoff) noexcept
{
    impl->tracefillgradient = onoff;
}
QPen ChartStyle::getTracePen() const
{
    QPen pen(impl->linecolor);
    if (impl->tracelinewidth > 0)
    {
        pen.setCosmetic(true);
        pen.setWidth(impl->tracelinewidth);
        pen.setStyle(Qt::PenStyle::SolidLine);
    }
    else
    {
        pen.setStyle(Qt::PenStyle::NoPen);
    }

    return pen;
}
QBrush ChartStyle::getTraceBrush() const
{
    QBrush brush;
    if (impl->tracefillopacity > 0)
    {
        auto clr = impl->fillcolor;
        clr.setAlpha(impl->tracefillopacity);
        brush.setColor(clr);
        brush.setStyle(Qt::BrushStyle::SolidPattern);
    }
    else
    {
        brush.setStyle(Qt::BrushStyle::NoBrush);
    }
    return brush;
}

void ChartStyle::connectToROIStyle(const ROIStyle* r)
{
    if (r != nullptr)
    {
        connect(r, &ROIStyle::StyleChanged, this, &ChartStyle::ROIStyleChanged);
        impl->linecolor = r->getLineColor();
        impl->fillcolor = r->getFillColor();
    }
}

void ChartStyle::ROIStyleChanged(const ROIStyle& r)
{
    impl->linecolor = r.getLineColor();
    impl->fillcolor = r.getFillColor();
    emit ColorChange();
}

void ChartStyle::setDoBackBrush(bool yesno) noexcept { impl->dobackbrush = yesno; }
bool ChartStyle::getDoBackBrush() const noexcept { return impl->dobackbrush; };

void ChartStyle::setNormalization(ROIVert::NORMALIZATION normalization) noexcept
{
    impl->normalization = normalization;
}
ROIVert::NORMALIZATION ChartStyle::getNormalization() const noexcept
{
    return impl->normalization;
}

void ChartStyle::setLimitStyle(ROIVert::LIMITSTYLE limitstyle) noexcept
{
    impl->limitstyle = limitstyle;
}
ROIVert::LIMITSTYLE ChartStyle::getLimitStyle() const noexcept
{
    return impl->limitstyle;
}

bool ChartStyle::getTraceFillGradient() const noexcept
{
    return impl->tracefillgradient;
}