#include "ROI\ROIStyle.h"
#include <functional>

struct ROIStyle::pimpl {
    QColor linecolor{ QColor(255,0,0) };
    QColor fillcolor{ QColor(255,0,0) };
    std::pair<QColor, QColor> selunselcolors = {QColor(0,255,0), QColor(255,0,0)};

    int linewidth{ 1 }; //3
    int fillopacity{ 60 };//0
    int selsize{ 15 };
    bool colorbyselected{ false };
    bool isSelected{ false };
    
    QColor getLineColor() const noexcept {
        return colorbyselected ? (isSelected ? selunselcolors.first : selunselcolors.second) : linecolor;
    }
    
    QColor getFillColor() const noexcept {
        return colorbyselected ? (isSelected ? selunselcolors.first : selunselcolors.second) : fillcolor;
    }

};
ROIStyle::ROIStyle() { }

// todo: write these copies better....
ROIStyle& ROIStyle::operator=(const ROIStyle& that) {
    if (this != &that) {    
        this->impl->linecolor = that.impl->linecolor;
        this->impl->fillcolor = that.impl->fillcolor;
        this->impl->linewidth = that.impl->linewidth;
        this->impl->fillopacity = that.impl->fillopacity;
        this->impl->selsize = that.impl->selsize;
        this->impl->colorbyselected = that.impl->colorbyselected;
        this->impl->selunselcolors = that.impl->selunselcolors;
    }
    emit this->StyleChanged(*this);
    return *this;
}

ROIStyle::ROIStyle(const ROIStyle& that) {
    if (this != &that) {    
        this->impl->linecolor = that.impl->linecolor;
        this->impl->fillcolor = that.impl->fillcolor;
        this->impl->linewidth = that.impl->linewidth;
        this->impl->fillopacity = that.impl->fillopacity;
        this->impl->selsize = that.impl->selsize;
        this->impl->colorbyselected = that.impl->colorbyselected;
        this->impl->selunselcolors = that.impl->selunselcolors;
    }
    emit this->StyleChanged(*this);
}

ROIStyle::~ROIStyle() = default;

QPen ROIStyle::getPen() const {
    QPen pen;
    pen.setCosmetic(true);
    pen.setColor(impl->getLineColor());
    if (impl->linewidth > 0) {
        pen.setWidth(impl->linewidth);
    }
    else {
        pen.setStyle(Qt::PenStyle::NoPen);
    }
    return pen;    
}
QBrush ROIStyle::getBrush() const {
    QBrush brush;
    if (impl->fillopacity > 0) {
        auto clr = impl->getFillColor();
        clr.setAlpha(impl->fillopacity);
        brush.setColor(clr);
        brush.setStyle(Qt::BrushStyle::SolidPattern);
    }
    else {
        brush.setStyle(Qt::BrushStyle::NoBrush);
    }
    return brush;
}

void ROIStyle::setColor(QColor color) {
    impl->linecolor = color;
    impl->fillcolor = color;
    
    emit StyleChanged(*this);
}
void ROIStyle::setLineColor(QColor color) {
    impl->linecolor = color;
    emit StyleChanged(*this);
}
QColor ROIStyle::getLineColor() const noexcept { return impl->linecolor; }
QColor ROIStyle::getFillColor() const noexcept { return impl->fillcolor; }
void ROIStyle::setFillColor(QColor color) {    
    impl->fillcolor = color;
    emit StyleChanged(*this);
}
void ROIStyle::setLineWidth(int linewidth) {    
    impl->linewidth = linewidth;
    emit StyleChanged(*this);
}
void ROIStyle::setFillOpacity(int opacity) {
    impl->fillopacity = opacity;
    emit StyleChanged(*this);
}
void ROIStyle::setSelectorSize(int size) {
    impl->selsize = size;
    emit StyleChanged(*this);

}
void ROIStyle::setColorBySelected(bool cbs) {
    impl->colorbyselected = cbs;
    emit StyleChanged(*this);
}
void ROIStyle::setSelectedColor(QColor color) {
    impl->selunselcolors.first = color;
    emit StyleChanged(*this);
}
void ROIStyle::setUnselectedColor(QColor color) {
    impl->selunselcolors.second = color;
    emit StyleChanged(*this);
}
int ROIStyle::getSelectorSize() const noexcept {
    return impl->selsize;
}
bool ROIStyle::isColorBySelected() const noexcept {
    return impl->colorbyselected;
}

void ROIStyle::setSelected(bool sel) {
    impl->isSelected = sel;
    if (impl->colorbyselected) {
        emit StyleChanged(*this);
    }
}


void ROIPalette::setPaletteColors(std::vector<QColor> clrs){
    palettecolors = clrs;
    emit paletteChanged();
}
std::vector<QColor> ROIPalette::getPaletteColors()  const noexcept{
    return palettecolors;
}

std::vector<QColor> ROIPalette::getPaletteColors(std::vector<size_t> inds)  const {
    std::vector<QColor> ret;
    if (!palettecolors.empty()) {
        std::transform(inds.begin(), inds.end(), ret.begin(), [&](size_t ind)->QColor {return palettecolors[ind % palettecolors.size()]; });
    }
    return ret;
}

QColor ROIPalette::getPaletteColor(size_t ind) const noexcept {
    auto ret = QColor();
    if (palettecolors.empty()) {
        return ret;
    }
    return palettecolors[ind % palettecolors.size()];
}