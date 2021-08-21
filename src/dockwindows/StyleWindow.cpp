#include "dockwindows/StyleWindow.h"

#include <QTabWidget>
#include <QGridLayout>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QFontDatabase>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDebug>

#include "widgets/RGBWidget.h"

struct StyleWindow::pimpl{
    QTabWidget* tab = new QTabWidget;

    RGBWidget* roicolor = new RGBWidget;

    QSpinBox* roilinewidth = new QSpinBox;
    QSlider* roiselsize = new QSlider;
    QSlider* roifillopacity = new QSlider;

    RGBWidget* chartforecolor = new RGBWidget;
    RGBWidget* chartbackcolor = new RGBWidget;

    QComboBox* chartfont = new QComboBox;
    QSpinBox* chartlabelfontsize = new QSpinBox;
    QSpinBox* charttickfontsize = new QSpinBox;

    QSpinBox* linewidth = new QSpinBox;
    QSlider* linefill = new QSlider;
    QCheckBox* linegradient = new QCheckBox;
    QCheckBox* linegrid = new QCheckBox;
    QCheckBox* linematchy = new QCheckBox;
    QComboBox* linenorm = new QComboBox;

    QSpinBox* ridgewidth = new QSpinBox;
    QSlider* ridgefill = new QSlider;
    QCheckBox* ridgegradient = new QCheckBox;
    QCheckBox* ridgegrid = new QCheckBox;
    QSlider* ridgeoverlap = new QSlider;

    void doLayout(){
        tab->tabBar()->setStyle(new CustomTabStyle);
        tab->setTabPosition(QTabWidget::TabPosition::West);

        // add tabs:
        tab->addTab(ROIColorTab(), "ROI Color");
        tab->addTab(ROIStyleTab(), "ROI Style");
        tab->addTab(ChartColorTab(), "Chart Colors");
        tab->addTab(ChartFontsTab(), "Chart Fonts");
        tab->addTab(ChartLineTab(), "Line Style");
        tab->addTab(ChartRidgeTab(), "Ridge Style");

        tab->setCurrentIndex(3);
    }

    QWidget* ROIColorTab(){
        auto ret = new QWidget;
        auto lay = new QVBoxLayout;
        ret->setLayout(lay);
        lay->addWidget(new QLabel("Applies to selected ROI(s)"));
        lay->addWidget(roicolor);
        lay->addStretch();
        return ret;
    }

    QWidget* ROIStyleTab(){
        auto ret = new QWidget;
        auto lay = new QFormLayout;
        ret->setLayout(lay);
        roiselsize->setMinimum(0);
        roiselsize->setMaximum(30);
        roiselsize->setOrientation(Qt::Horizontal);
        roifillopacity->setMinimum(0);
        roifillopacity->setMaximum(100); //todo:255?
        roifillopacity->setOrientation(Qt::Horizontal);

        lay->addRow("Line Width", roilinewidth);
        lay->addRow("Selecter Size", roiselsize);
        lay->addRow("Fill Opacity", roifillopacity);
        return ret;
    }
    QWidget* ChartColorTab(){
        auto ret = new QWidget;
        auto lay = new QVBoxLayout;
        ret->setLayout(lay);
        lay->addWidget(new QLabel("Chart Foreground Color:"));
        lay->addWidget(chartforecolor);
        lay->addWidget(new QLabel("Chart Background Color:"));
        lay->addWidget(chartbackcolor);
        lay->addStretch(1);
        return ret;
    }
    QWidget* ChartFontsTab(){
        auto ret = new QWidget;
        auto lay = new QFormLayout;
        ret->setLayout(lay);
        QFontDatabase fontdb;
        for(auto &family : fontdb.families())  {
            chartfont->addItem(family);
        }
        chartfont->setMinimumWidth(150);

        chartlabelfontsize->setMinimum(1);
        chartlabelfontsize->setMinimum(72);
        charttickfontsize->setMinimum(1);
        charttickfontsize->setMinimum(72);

        lay->addRow("Font:", chartfont);
        lay->addRow("Label Size:", chartlabelfontsize);
        lay->addRow("Tick Size:", charttickfontsize);
        return ret;
    }

    QWidget* ChartLineTab(){
        auto ret = new QWidget;
        auto lay = new QFormLayout;
        ret->setLayout(lay);
        linewidth->setMinimum(1);
        linewidth->setMaximum(10);
        linefill->setMinimum(0);
        linefill->setMaximum(100);
        linefill->setOrientation(Qt::Horizontal);
        linenorm->addItems({"None", "Zero to One", "L1 Norm", "L2 Norm", "Z Score", "Median IQR"});

        lay->addRow("Line Width:", linewidth);
        lay->addRow("Fill Opacity:", linefill);
        lay->addRow("Fill Gradient:", linegradient);
        lay->addRow("Grid:", linegrid);
        lay->addRow("Match Y Axes:", linematchy);
        lay->addRow("Normalization:", linenorm);
        return ret;
    }
    QWidget* ChartRidgeTab(){
        auto ret = new QWidget;
        auto lay = new QFormLayout;
        ret->setLayout(lay);
        ridgewidth->setMinimum(1);
        ridgewidth->setMaximum(10);
        ridgefill->setMinimum(0);
        ridgefill->setMaximum(100);
        ridgefill->setOrientation(Qt::Horizontal);
        ridgeoverlap->setMinimum(0);
        ridgeoverlap->setMaximum(100);
        ridgeoverlap->setOrientation(Qt::Horizontal);
        lay->addRow("Line Width:", ridgewidth);
        lay->addRow("Fill Opacity:", ridgefill);
        lay->addRow("Fill Gradient:", ridgegradient);
        lay->addRow("Grid:", ridgegrid);
        lay->addRow("Overlap:", ridgeoverlap);
        return ret;
    }

    void doConnect(StyleWindow* par){
        connect(roicolor, &RGBWidget::colorChanged, par, &StyleWindow::ROIColorChange);

        connect(roilinewidth, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWindow::ROIStyleChange);
        connect(roiselsize, &QSlider::valueChanged, par, &StyleWindow::ROIStyleChange);
        connect(roifillopacity, &QSlider::valueChanged, par, &StyleWindow::ROIStyleChange);
        connect(chartforecolor, &RGBWidget::colorChanged, par, &StyleWindow::ChartStyleChange);
        connect(chartbackcolor, &RGBWidget::colorChanged, par, &StyleWindow::ChartStyleChange);
        connect(chartfont, QOverload<int>::of(&QComboBox::currentIndexChanged), par, &StyleWindow::ChartStyleChange);
        connect(chartlabelfontsize, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWindow::ChartStyleChange);
        connect(charttickfontsize, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWindow::ChartStyleChange);

        connect(linewidth, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWindow::LineChartStyleChange);
        connect(linefill, &QSlider::valueChanged, par, &StyleWindow::LineChartStyleChange);
        connect(linegradient, &QCheckBox::stateChanged, par, &StyleWindow::LineChartStyleChange);
        connect(linegrid, &QCheckBox::stateChanged, par, &StyleWindow::LineChartStyleChange);
        connect(linematchy, &QCheckBox::stateChanged, par, &StyleWindow::LineChartStyleChange);
        connect(linenorm, QOverload<int>::of(&QComboBox::currentIndexChanged), par, &StyleWindow::LineChartStyleChange);

        connect(ridgewidth, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWindow::RidgeChartStyleChange);
        connect(ridgefill, &QSlider::valueChanged, par, &StyleWindow::RidgeChartStyleChange);
        connect(ridgegrid, &QCheckBox::stateChanged, par, &StyleWindow::RidgeChartStyleChange);
        connect(ridgeoverlap, &QSlider::valueChanged, par, &StyleWindow::RidgeChartStyleChange);
    }
};

StyleWindow::StyleWindow(QWidget *parent) : QWidget(parent)
{
    auto lay = new QGridLayout(this);
    lay->addWidget(impl->tab);
    impl->doLayout();
    impl->doConnect(this);
}
void StyleWindow::ROIColorChange() {
    emit ROIColorChanged(impl->roicolor->getColor());
}

void StyleWindow::ROIStyleChange(){
    qDebug()<<"ROIStyleChange";
}
void StyleWindow::ChartStyleChange(){
    qDebug()<<"ChartStyleChange";

}
void StyleWindow::LineChartStyleChange(){
    qDebug()<<"LineChartStyleChange";
}
void StyleWindow::RidgeChartStyleChange(){
    qDebug()<<"RidgeChartStyleChange";
}
