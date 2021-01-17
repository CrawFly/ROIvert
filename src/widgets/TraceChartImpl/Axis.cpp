#include "widgets\TraceChartImpl\Axis.h"
#include "widgets\TraceChartImpl\Core.h"

#include <QRect>
#include <QStringList>
#include <QFont>
#include <QPainter>
#include <cmath>
#include <QDebug>

using namespace TraceChart;

struct Axis::pimpl {
    pimpl() {
        labelfontmea = QFontMetrics(style.LabelFont);
        tickfontmea = QFontMetrics(style.TickLabelFont);
    }

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

    QFontMetrics labelfontmea = QFontMetrics(QFont());
    QFontMetrics tickfontmea = QFontMetrics(QFont());

    std::vector<double> tickvalues;
    std::vector<double> tickwidths;
    QStringList tickstrings;

    AxisStyle style;

    AxisStyle getStyle() {
        return style;
    }
    void setStyle(AxisStyle s) {
        style = s;
        labelfontmea = QFontMetrics(style.LabelFont);
        tickfontmea = QFontMetrics(style.TickLabelFont);
    }
    int thickness() noexcept {
        return labelthickness + spacelabel + ticklabelthickness + spaceticklabel + spacetickmark + ticklength;
    }
};
Axis::Axis() {
    updateLayout();
}
Axis::~Axis() = default;

void Axis::setExtents(const double& min, const double& max) {
    if (max > min) {
        impl->extents = std::make_tuple(min, max);
        updateLayout();
    }
}
std::tuple<double, double> Axis::getExtents() const  noexcept {
    return impl->extents;
}
std::tuple<double, double> Axis::getLimits() const {

    double min = impl->tightleft ? std::get<0>(impl->extents) : impl->tickvalues.front();
    double max = impl->tightright ? std::get<1>(impl->extents) : impl->tickvalues.back();

    return std::make_tuple(min, max);
}

void Axis::setLabel(const QString & Label) {
    impl->label = Label;
    updateLayout();
}
QString Axis::getLabel() const noexcept {
    return impl->label;
}

void Axis::setZero(const int& xzero, const int& yzero) noexcept {
    impl->position.setRect(xzero, yzero, impl->position.width(), impl->position.height());
}
void Axis::setSpacings(const int& label, const int& ticklabel, const int& tickmark) noexcept {
    impl->spacelabel = label;
    impl->spaceticklabel = ticklabel;
    impl->spacetickmark = tickmark;
}
void Axis::setTickLength(const int& ticklength) noexcept {
    impl->ticklength = ticklength;
}
void Axis::setMaxNTicks(const unsigned int& n) {
    const auto old = impl->maxnticks;
    impl->maxnticks = std::clamp((int)n, 3, 50);

    if (old != impl->maxnticks) {
        updateLayout();
    }
}

void Axis::setStyle(AxisStyle s) {
    impl->setStyle(s);
    updateLayout();
}
AxisStyle Axis::getStyle() {
    return impl->getStyle();
}




void Axis::updateLayout() {

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
        impl->tickvalues = tickpicker::getNiceTicksLimits(extmin, extmax, impl->maxnticks);
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

void HAxis::paint(QPainter & painter) {
    const QRect& pos = impl->position;

    painter.setPen(impl->style.Color);
    painter.setBrush(Qt::NoBrush);

    // Label:
    painter.setFont(impl->style.LabelFont);
    const QRect labelR(pos.left(), pos.bottom() - impl->spacelabel - impl->labelthickness, pos.width(), impl->labelthickness);
    painter.drawText(labelR, impl->label, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));

    // Tick Labels:
    painter.setFont(impl->style.TickLabelFont);
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
void HAxis::updateLayout() {
    Axis::updateLayout();
    impl->ticklabelthickness = impl->tickfontmea.height();

    // set the thickness to the sumb of everything:
    impl->position.setHeight(impl->thickness());
}
void HAxis::setLength(const int& length) noexcept {
    impl->position.setWidth(length);
}
int HAxis::getLength() const noexcept {
    return impl->position.width();
}
std::tuple<double, double> HAxis::getMargins() const {
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
int HAxis::getThickness() const noexcept {
    return impl->position.height();
}

void VAxis::paint(QPainter & painter) {
    const QRect& pos = impl->position;
    painter.setPen(impl->style.Color);
    painter.setBrush(Qt::NoBrush);

    painter.setFont(impl->getStyle().LabelFont);

    const QPointF point(pos.left() + impl->spacelabel, pos.top() + pos.height() / 2.);

    painter.save();
    painter.translate(point);
    painter.rotate(-90);
    const QRect labelR(-pos.height() / 2, 0, pos.height(), impl->labelthickness);
    painter.drawText(labelR, impl->label, QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    painter.restore();

    //
    painter.setFont(impl->getStyle().TickLabelFont);
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
void VAxis::updateLayout() {
    Axis::updateLayout();

    impl->ticklabelthickness = 0.;
    impl->ticklabelthickness = *std::max_element(impl->tickwidths.begin(), impl->tickwidths.end());

    impl->position.setWidth(impl->thickness());
}
void VAxis::setLength(const int& length) noexcept {
    impl->position.setHeight(length);
}
int VAxis::getLength() const noexcept {
    return impl->position.height();
}
std::tuple<double, double> VAxis::getMargins() const {
    double marg = static_cast<double>(impl->tickfontmea.height()) / 2.;
    return std::make_tuple(marg, marg);
}
int VAxis::getThickness() const noexcept {
    return impl->position.width();
}

