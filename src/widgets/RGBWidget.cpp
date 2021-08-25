#include "widgets/RGBWidget.h"
#include <QSlider>
#include <QGridLayout>
#include <QSpinBox>

struct RGBWidget::pimpl{
    std::vector<std::pair<QSlider*, QSpinBox*>> rgb;
};

RGBWidget::RGBWidget(QWidget *parent) : QWidget(parent)
{
    auto grid = new QGridLayout(this);

    for (size_t i = 0; i < 3 ; ++i) {
        auto slide = new QSlider(Qt::Orientation::Horizontal,this);
        auto spin = new QSpinBox(this);
        slide->setMinimum(0);
        slide->setMaximum(255);
        slide->setTickInterval(1);
        spin->setMinimum(0);
        spin->setMaximum(255);


        grid->addWidget(slide,(int)i,1);
        grid->addWidget(spin,(int)i,2);
        connect(slide, &QSlider::valueChanged, spin, &QSpinBox::setValue);
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), slide, &QSlider::setValue);

        connect(slide, &QSlider::valueChanged, this, [=](int){emit colorChanged(getColor());});

        impl->rgb.push_back({slide,spin});
    }

    impl->rgb[0].first->setStyleSheet("QSlider::handle:horizontal {background: #f99; border: 3px solid #f00; width: 13px;margin-top: -2px;margin-bottom: -2px;border-radius: 6px;}");
    impl->rgb[1].first->setStyleSheet("QSlider::handle:horizontal {background: #9f9; border: 3px solid #0f0; width: 13px;margin-top: -2px;margin-bottom: -2px;border-radius: 6px;}");
    impl->rgb[2].first->setStyleSheet("QSlider::handle:horizontal {background: #99f; border: 3px solid #00f; width: 13px;margin-top: -2px;margin-bottom: -2px;border-radius: 6px;}");

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //this->setMinimumHeight(100);
    this->setFixedHeight(120);
}
QColor RGBWidget::getColor() const {
    const int r{ impl->rgb[0].second->value() };
    const int g{ impl->rgb[1].second->value() };
    const int b{ impl->rgb[2].second->value() };
    return QColor(r,g,b);
}

void RGBWidget::setColor(const QColor &clr){
    this->blockSignals(true);
    impl->rgb[0].first->setValue(clr.red());
    impl->rgb[1].first->setValue(clr.green());
    impl->rgb[2].first->setValue(clr.blue());
    this->blockSignals(false);
    emit colorChanged(getColor());
}