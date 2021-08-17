#include "widgets\TraceChartWidget.h"
#include <QPainter>
#include "ChartStyle.h"
namespace {
    double niceNum(const double& range, const bool round) noexcept {
        float exponent = std::floor(std::log10(range));
        float fraction = range / std::pow(10.f, exponent);
        float niceFraction = 0.; /** nice, rounded fraction */

        if (round)
        {
            if (fraction < 1.5)
                niceFraction = 1;
            else if (fraction < 3)
                niceFraction = 2;
            else if (fraction < 7)
                niceFraction = 5;
            else
                niceFraction = 10;
        }
        else
        {
            if (fraction <= 1)
                niceFraction = 1;
            else if (fraction <= 2)
                niceFraction = 2;
            else if (fraction <= 5)
                niceFraction = 5;
            else
                niceFraction = 10;
        }

        return niceFraction * pow(10, exponent);
    }
    std::vector<double> getNiceTicksLimits(double minval, const double& maxval, const unsigned int& maxticks) {
        // https://stackoverflow.com/questions/8506881/nice-label-algorithm-for-charts-with-minimum-ticks
        // might still be some bugs here around floating point precision, hard to fgigure out how to not make a mess out of this!

        const auto range = niceNum(maxval - minval, false);
        const auto spacing = niceNum(range / (maxticks - 1), true);

        //spacing = std::round(spacing / spacing_round_fac) * spacing_round_fac;

        auto nicemin = std::floor(minval / spacing) * spacing;
        const auto nicemax = ceil(maxval / spacing) * spacing;

        // rfac is an attempt at dealing with floating point precision issues:
        //   when floor jumps down too much, increment nicemin by one spacing unit
        //      similar for maxval..
        const double rfac = std::pow(10, std::floor(log10(spacing)) - 5);

        if (std::abs(minval - (nicemin + spacing)) < rfac) {
            nicemin += spacing;
        }

        std::vector<double> ticks = { nicemin };

        while ((maxval - ticks.back()) > rfac) {
            ticks.push_back(ticks.back() + spacing);
        }
        return ticks;
    };
}



struct TraceChartAxis::pimpl {
    QString label;

    QRect position = QRect(0, 0, 1, 1);
    std::tuple<qreal, qreal> extents = std::make_tuple(0, 1);;

    int spacelabel = 0, spaceticklabel = 10, spacetickmark = 5;
    int labelthickness = 0, ticklabelthickness = 0; //** these are font size caches...
    int margins[2] = { 0,0 };

    int ticklength = 8;

    int maxnticks = 14;

    bool tightleft = false;
    bool tightright = false;

    bool visible = true;

    QFontMetrics labelfontmea = QFontMetrics(QFont());
    QFontMetrics tickfontmea = QFontMetrics(QFont());

    std::vector<double> tickvalues;
    std::vector<double> tickwidths;
    QStringList tickstrings;

    int thickness() noexcept {
        return labelthickness + spacelabel + ticklabelthickness + spaceticklabel + spacetickmark + ticklength;
    }
    std::shared_ptr<ChartStyle> chartstyle;
};
TraceChartAxis::TraceChartAxis(std::shared_ptr<ChartStyle> style) {
    setStyle(style);
    updateLayout();
}
TraceChartAxis::~TraceChartAxis() = default;
void TraceChartAxis::setStyle(std::shared_ptr<ChartStyle> style) {
    impl->chartstyle = style;
}
void TraceChartAxis::setExtents(const double& min, const double& max) {
    if (max > min) {
        impl->extents = std::make_tuple(min, max);
        updateLayout();
    }
}
std::tuple<double, double> TraceChartAxis::getExtents() const  noexcept {
    return impl->extents;
}
std::tuple<double, double> TraceChartAxis::getLimits() const {

    double min = impl->tightleft ? std::get<0>(impl->extents) : impl->tickvalues.front();
    double max = impl->tightright ? std::get<1>(impl->extents) : impl->tickvalues.back();

    return std::make_tuple(min, max);
}

void TraceChartAxis::setLabel(const QString & Label) {
    impl->label = Label;
    updateLayout();
}
QString TraceChartAxis::getLabel() const noexcept {
    return impl->label;
}

void TraceChartAxis::setZero(const int& xzero, const int& yzero) noexcept {
    impl->position.setRect(xzero, yzero, impl->position.width(), impl->position.height());
}
void TraceChartAxis::setSpacings(const int& label, const int& ticklabel, const int& tickmark) noexcept {
    impl->spacelabel = label;
    impl->spaceticklabel = ticklabel;
    impl->spacetickmark = tickmark;
}
void TraceChartAxis::setTickLength(const int& ticklength) noexcept {
    impl->ticklength = ticklength;
}
void TraceChartAxis::setMaxNTicks(const unsigned int& n) {
    const auto old = impl->maxnticks;
    impl->maxnticks = std::clamp((int)n, 3, 50);

    if (old != impl->maxnticks) {
        updateLayout();
    }
}

void TraceChartAxis::updateLayout() {
    double extmin, extmax;
    std::tie(extmin, extmax) = impl->extents;

    // hacky fix for rounding errors?
    if ((extmax - extmin) > 1E-5) {
        extmax = std::round(extmax * 1E10) / 1E10;
        extmin = std::round(extmin * 1E10) / 1E10;
    }

    // we maybe bail on the tickpicker if the range is crazy?
    if ((extmin != 0 && std::abs(log10(extmin)) > 7) || (extmax != 0 && std::abs(log10(extmax)) > 7)) {
        impl->tickvalues = { extmin,(extmin + extmax) / 2, extmax };
    }
    else {
        impl->tickvalues = getNiceTicksLimits(extmin, extmax, impl->maxnticks);
    }


    impl->tightleft = (extmin - impl->tickvalues.front()) / (extmax - extmin) > .05;//&& impl->tickvalues.size() > 3;
    impl->tightright = (impl->tickvalues.back() - extmax) / (extmax - extmin) > .05;//&& impl->tickvalues.size() > 3;

    // cast to string for tickstrings
    impl->tickstrings.clear();
    impl->tickwidths.clear();

    for (auto val : impl->tickvalues) {
        // *** note, don't add a reference to this loop!
        val = std::round(val * 1E10) / 1E10;

        QString str(QString::number(val, 'G', 7).replace("-", QChar(0x2212)));
        impl->tickstrings.push_back(str);
        impl->tickwidths.push_back(impl->tickfontmea.size(Qt::TextSingleLine, str).width());
    }

    impl->labelthickness = 0;
    if (!impl->label.isEmpty()) {
        impl->labelthickness = impl->labelfontmea.height();
    }
}
void TraceChartAxis::setVisible(bool yesno) noexcept { impl->visible = yesno; }
bool TraceChartAxis::getVisible() const noexcept { return impl->visible; }

TraceChartHAxis::TraceChartHAxis(std::shared_ptr<ChartStyle> style) : TraceChartAxis(style) {};
void TraceChartHAxis::paint(QPainter & painter) {
    if (!impl->visible || impl->chartstyle==nullptr) {
        return;
    }

    const QRect& pos = impl->position;

    
    
    painter.setPen(impl->chartstyle->getAxisPen());
    painter.setBrush(Qt::NoBrush);

    // Label:
    // todo: font size in style
    //painter.setFont(impl->style.LabelFont);
    painter.setFont(QFont());

    const QRect labelR(pos.left(), pos.bottom() - impl->spacelabel - impl->labelthickness, pos.width(), impl->labelthickness);
    painter.drawText(labelR, impl->label, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));

    // Tick Labels:
    // todo: font size in style
    //painter.setFont(impl->style.TickLabelFont);
    const int ticklabeltop = labelR.top() - impl->spaceticklabel - impl->ticklabelthickness;

    double min, max;
    std::tie(min, max) = getLimits();
    const double span = max - min;

    for (int i = impl->tightleft; i < impl->tickvalues.size() - impl->tightright; ++i) {
        const float propalongruler = (impl->tickvalues[i] - min) / (span);
        const int xpos = pos.left() + propalongruler * pos.width();
        const int w = impl->tickwidths[i];
        const QRect ticklabelR(xpos - w / 2, ticklabeltop, w, impl->ticklabelthickness);
        painter.drawText(ticklabelR, impl->tickstrings[i], QTextOption(Qt::AlignHCenter | Qt::AlignTop));

        // tick marks:
        painter.drawLine(xpos, pos.top(), xpos, pos.top() + impl->ticklength);
    }

    // Axle:
    painter.drawLine(pos.topLeft(), pos.topRight());
}
void TraceChartHAxis::updateLayout() {
    TraceChartAxis::updateLayout();
    impl->ticklabelthickness = impl->tickfontmea.height();

    // set the thickness to the sumb of everything:
    impl->position.setHeight(impl->thickness());
}
void TraceChartHAxis::setLength(const int& length) noexcept {
    impl->position.setWidth(length);
}
int TraceChartHAxis::getLength() const noexcept {
    return impl->position.width();
}
std::tuple<double, double> TraceChartHAxis::getMargins() const {
    // margins for HAxis come from the first and last tick labels. 
    double left = 0., right = 0.;
    if (!impl->tickstrings.empty()) {
        const int wL = impl->tickfontmea.size(Qt::TextSingleLine, impl->tickstrings.first()).width();
        const int wR = impl->tickfontmea.size(Qt::TextSingleLine, impl->tickstrings.last()).width();
        // this is imperfect for the trimmed case... that's okay?

        left = static_cast<double>(wL) / 2.;
        right = static_cast<double>(wR) / 2.;
    }
    return std::make_tuple(left, right);
}
int TraceChartHAxis::getThickness() const noexcept {
    return impl->visible ? impl->position.height() : 0;
}

TraceChartVAxis::TraceChartVAxis(std::shared_ptr<ChartStyle> style) : TraceChartAxis(style) {};
void TraceChartVAxis::paint(QPainter & painter) {
    if (!impl->visible || impl->chartstyle==nullptr) {
        return;
    }
    const QRect& pos = impl->position;
    painter.setPen(impl->chartstyle->getAxisPen());
    painter.setBrush(Qt::NoBrush);

    // todo: font
    //painter.setFont(impl->getStyle().LabelFont);
    painter.setFont(QFont());

    const QPointF point(pos.left() + impl->spacelabel, pos.top() + pos.height() / 2.);

    painter.save();
    painter.translate(point);
    painter.rotate(-90);
    const QRect labelR(-pos.height() / 2, 0, pos.height(), impl->labelthickness);
    painter.drawText(labelR, impl->label, QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    painter.restore();

    //
    //painter.setFont(impl->getStyle().TickLabelFont);
    const int xpos = pos.left() + impl->spacelabel + impl->labelthickness + impl->spaceticklabel;
    const int height = impl->tickfontmea.height();
    const int w = impl->ticklabelthickness;

    double min, max;
    std::tie(min, max) = getLimits();
    const double span = max - min;

    for (int i = impl->tightleft; i < impl->tickvalues.size() - impl->tightright; ++i) {
        const float propalongruler = (impl->tickvalues[i] - min) / (span);
        const int ypos = pos.bottom() - propalongruler * pos.height();
        const QRect ticklabelR(xpos, ypos - height / 2, w, height);
        painter.drawText(ticklabelR, impl->tickstrings[i], QTextOption(Qt::AlignRight | Qt::AlignVCenter));

        // tick marks:
        painter.drawLine(pos.right(), ypos, pos.right() - impl->ticklength, ypos);
    }

    painter.drawLine(pos.topRight(), pos.bottomRight());

}
void TraceChartVAxis::updateLayout() {
    TraceChartAxis::updateLayout();

    impl->ticklabelthickness = 0.;
    impl->ticklabelthickness = *std::max_element(impl->tickwidths.begin(), impl->tickwidths.end());

    impl->position.setWidth(impl->thickness());
}
void TraceChartVAxis::setLength(const int& length) noexcept {
    impl->position.setHeight(length);
}
int TraceChartVAxis::getLength() const noexcept {
    return impl->position.height();
}
std::tuple<double, double> TraceChartVAxis::getMargins() const {
    double marg = static_cast<double>(impl->tickfontmea.height()) / 2.;
    return std::make_tuple(marg, marg);
}
int TraceChartVAxis::getThickness() const noexcept {
    return impl->visible ? impl->position.width() : 0;
}

