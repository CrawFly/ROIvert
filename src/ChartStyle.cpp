#include "ChartStyle.h"

struct ChartStyle::pimpl {
    QColor backgroundcolor{ Qt::black };
    QColor axiscolor{ Qt::white };
    int axislinewidth{ 1 };
    bool grid = false;
    int titlefontsize{ 14 };
    int labelfontsize{ 12 };
    int tickfontsize{ 10 };

    int tracelinewidth{ 3 };
    int tracefillopacity{ 55 };
    bool tracefillgradient{ false };
    QColor linecolor{ Qt::red };
    QColor fillcolor{ Qt::red };
};

ChartStyle::ChartStyle() = default;

ChartStyle::~ChartStyle() {}

ChartStyle& ChartStyle::operator=(const ChartStyle& that) {
    if (this != &that) {
        *(impl) = *(that.impl);
    }
    return *this;
}

ChartStyle::ChartStyle(const ChartStyle& that) {
    if (&that!=nullptr && that.impl!=nullptr && this != &that) {
        *(impl) = *(that.impl);
    }
}

// Charts
void ChartStyle::setBackgroundColor(QColor c) {
    impl->backgroundcolor = c;
    emit StyleChanged(*this);
}
QColor ChartStyle::getBackgroundColor() const noexcept {
    return impl->backgroundcolor;
}

// Axes
void ChartStyle::setAxisColor(QColor c) {
    impl->axiscolor = c;
    emit StyleChanged(*this);
}
void ChartStyle::setAxisLineWidth(int w) {
    impl->axislinewidth = w;
    emit StyleChanged(*this);
}
void ChartStyle::setGrid(bool onoff) {
    impl->grid = onoff;
    emit StyleChanged(*this);
}
void ChartStyle::setTitleFontSize(int fs) {
    impl->titlefontsize = fs;
    emit StyleChanged(*this);
}
void ChartStyle::setLabelFontSize(int fs) {
    impl->labelfontsize = fs;
    emit StyleChanged(*this);
}
void ChartStyle::setTickLabelFontSize(int fs) {
    impl->tickfontsize = fs;
    emit StyleChanged(*this);
}
    
QPen ChartStyle::getAxisPen() const {
    return QPen(impl->axiscolor, impl->axislinewidth);
}
bool ChartStyle::getGrid() const noexcept {
    return impl->grid;
}
// Traces
void ChartStyle::setTraceLineWidth(int w) {
    impl->tracelinewidth = w;
    emit StyleChanged(*this);
}
void ChartStyle::setTraceFillOpacity(int o) {
    impl->tracefillopacity = o;
    emit StyleChanged(*this);
}
void ChartStyle::setTraceFillGradient(bool onoff) {
    impl->tracefillgradient = onoff;
    emit StyleChanged(*this);
}
QPen ChartStyle::getTracePen() const {
    QPen pen(impl->linecolor);
    if (impl->tracelinewidth > 0) {
        pen.setCosmetic(true);
        pen.setWidth(impl->tracelinewidth);
        pen.setStyle(Qt::PenStyle::SolidLine);
    }
    else {
        pen.setStyle(Qt::PenStyle::NoPen);
    }
    
    return pen;
}
QBrush ChartStyle::getTraceBrush() const {
    QBrush brush;
    if (impl->tracefillopacity > 0) {
        auto clr = impl->fillcolor;
        clr.setAlpha(impl->tracefillopacity);
        brush.setColor(clr);
        brush.setStyle(Qt::BrushStyle::SolidPattern);
    }
    else {
        brush.setStyle(Qt::BrushStyle::NoBrush);
    }
    return brush;
}

void ChartStyle::connectToROIStyle(ROIStyle* r) {
    connect(r, &ROIStyle::StyleChanged, this, &ChartStyle::ROIStyleChanged);
    impl->linecolor = r->getLineColor();
    impl->fillcolor = r->getFillColor();
}

void ChartStyle::ROIStyleChanged(const ROIStyle& r) {
    impl->linecolor = r.getLineColor();
    impl->fillcolor = r.getFillColor();
    emit StyleChanged(*this);
}