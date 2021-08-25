#include "dockwidgets/StyleWidget.h"

#include <QDebug>

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>

#include "ROI/ROIs.h"
#include "TraceView.h"
#include "ROI/ROIStyle.h"
#include "ChartStyle.h"
#include "widgets/RGBWidget.h"
#include "widgets/TraceChartWidget.h"

struct StyleWidget::pimpl {
    QTabWidget* tab = new QTabWidget;

    RGBWidget* roicolor = new RGBWidget;

    QSlider* roilinewidth = new QSlider;
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


    ROIs* rois{ nullptr };
    TraceView* traceview{ nullptr };

    void doLayout() {
        tab->tabBar()->setStyle(new CustomTabStyle);
        tab->setTabPosition(QTabWidget::TabPosition::West);

        // add tabs:
        tab->addTab(ROIColorTab(), "ROI Color");
        tab->addTab(ROIStyleTab(), "ROI Style");
        tab->addTab(ChartColorTab(), "Chart Colors");
        tab->addTab(ChartFontsTab(), "Chart Fonts");
        tab->addTab(ChartLineTab(), "Line Style");
        tab->addTab(ChartRidgeTab(), "Ridge Style");

        tab->setCurrentIndex(0);
    }

    QWidget* ROIColorTab() {
        auto ret = new QWidget;
        auto lay = new QVBoxLayout;
        ret->setLayout(lay);
        lay->addWidget(new QLabel("Applies to selected ROI(s)"));
        lay->addWidget(roicolor);
        lay->addStretch();
        return ret;
    }
    QWidget* ROIStyleTab() {
        auto ret = new QWidget;
        auto lay = new QFormLayout;
        ret->setLayout(lay);
        roilinewidth->setMinimum(0);
        roilinewidth->setMaximum(15);
        roilinewidth->setOrientation(Qt::Horizontal);
        roilinewidth->setPageStep(1);
        roilinewidth->setSingleStep(1);

        roiselsize->setMinimum(0);
        roiselsize->setMaximum(30);
        roiselsize->setOrientation(Qt::Horizontal);
        roiselsize->setPageStep(1);
        roiselsize->setSingleStep(1);

        roifillopacity->setMinimum(0);
        roifillopacity->setMaximum(255);
        roifillopacity->setOrientation(Qt::Horizontal);
        roifillopacity->setPageStep(8);
        roifillopacity->setSingleStep(1);

        lay->addRow("Line Width", roilinewidth);
        lay->addRow("Selecter Size", roiselsize);
        lay->addRow("Fill Opacity", roifillopacity);
        return ret;
    }
    QWidget* ChartColorTab() {
        auto ret = new QWidget;
        auto lay = new QVBoxLayout;
        ret->setLayout(lay);
        lay->addWidget(new QLabel("Chart Foreground:"));
        lay->addWidget(chartforecolor);
        lay->addWidget(new QLabel("Chart Background:"));
        lay->addWidget(chartbackcolor);
        lay->addStretch(1);
        return ret;
    }
    QWidget* ChartFontsTab() {
        auto ret = new QWidget;
        auto lay = new QFormLayout;
        ret->setLayout(lay);
        QFontDatabase fontdb;
        for (auto& family : fontdb.families()) {
            chartfont->addItem(family);
        }
        chartfont->setMinimumWidth(150);

        chartlabelfontsize->setMinimum(1);
        chartlabelfontsize->setMaximum(72);
        charttickfontsize->setMinimum(1);
        charttickfontsize->setMaximum(72);

        lay->addRow("Font:", chartfont);
        lay->addRow("Label Size:", chartlabelfontsize);
        lay->addRow("Tick Size:", charttickfontsize);
        return ret;
    }

    QWidget* ChartLineTab() {
        auto ret = new QWidget;
        auto lay = new QFormLayout;
        ret->setLayout(lay);
        linewidth->setMinimum(0);
        linewidth->setMaximum(15);
        linefill->setMinimum(0);
        linefill->setMaximum(255);
        linefill->setOrientation(Qt::Horizontal);
        linenorm->addItems({ "None", "Zero to One", "L1 Norm", "L2 Norm", "Z Score", "Median IQR" });

        lay->addRow("Line Width:", linewidth);
        lay->addRow("Fill Opacity:", linefill);
        lay->addRow("Fill Gradient:", linegradient);
        lay->addRow("Grid:", linegrid);
        lay->addRow("Match Y Axes:", linematchy);
        lay->addRow("Normalization:", linenorm);
        return ret;
    }
    QWidget* ChartRidgeTab() {
        auto ret = new QWidget;
        auto lay = new QFormLayout;
        ret->setLayout(lay);
        ridgewidth->setMinimum(0);
        ridgewidth->setMaximum(15);
        ridgefill->setMinimum(0);
        ridgefill->setMaximum(255);
        ridgefill->setOrientation(Qt::Horizontal);
        ridgeoverlap->setMinimum(0);
        ridgeoverlap->setMaximum(100);
        ridgeoverlap->setOrientation(Qt::Horizontal);
        lay->addRow("Line Width:", ridgewidth);
        lay->addRow("Fill Opacity:", ridgefill);
        lay->addRow("Fill Gradient:", ridgegradient);
        lay->addRow("Grid:", ridgegrid);
        lay->addRow("Offset:", ridgeoverlap);
        return ret;
    }

    void doConnect(const StyleWidget* const par) {
        if (par == nullptr) {
            return;
        }
        connect(roicolor, &RGBWidget::colorChanged, par, &StyleWidget::ROIColorChange);
        connect(roilinewidth, &QSlider::valueChanged, par, &StyleWidget::ROIStyleChange);
        connect(roiselsize, &QSlider::valueChanged, par, &StyleWidget::ROIStyleChange);
        connect(roifillopacity, &QSlider::valueChanged, par, &StyleWidget::ROIStyleChange);
        connect(chartforecolor, &RGBWidget::colorChanged, par, &StyleWidget::ChartStyleChange);
        connect(chartbackcolor, &RGBWidget::colorChanged, par, &StyleWidget::ChartStyleChange);
        connect(chartfont, QOverload<int>::of(&QComboBox::currentIndexChanged), par, &StyleWidget::ChartStyleChange);
        connect(chartlabelfontsize, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWidget::ChartStyleChange);
        connect(charttickfontsize, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWidget::ChartStyleChange);

        connect(linewidth, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWidget::LineChartStyleChange);
        connect(linefill, &QSlider::valueChanged, par, &StyleWidget::LineChartStyleChange);
        connect(linegradient, &QCheckBox::stateChanged, par, &StyleWidget::LineChartStyleChange);
        connect(linegrid, &QCheckBox::stateChanged, par, &StyleWidget::LineChartStyleChange);
        connect(linematchy, &QCheckBox::stateChanged, par, &StyleWidget::LineMatchyChange);
        connect(linenorm, QOverload<int>::of(&QComboBox::currentIndexChanged), par, &StyleWidget::LineChartStyleChange);

        connect(ridgewidth, QOverload<int>::of(&QSpinBox::valueChanged), par, &StyleWidget::RidgeChartStyleChange);
        connect(ridgegradient, &QCheckBox::stateChanged, par, &StyleWidget::RidgeChartStyleChange);
        connect(ridgefill, &QSlider::valueChanged, par, &StyleWidget::RidgeChartStyleChange);
        connect(ridgegrid, &QCheckBox::stateChanged, par, &StyleWidget::RidgeChartStyleChange);
        connect(ridgeoverlap, &QSlider::valueChanged, par, &StyleWidget::RidgeOverlapChange);
    }

    void updateROIStyle(ROIStyle* style) {
        if (!isLoading) {
            auto selsize = roiselsize->value() > 0 ? roiselsize->value() : -15;
            style->blockSignals(true);
            style->setSelectorSize(selsize);
            style->setLineWidth(roilinewidth->value());
            style->setFillOpacity(roifillopacity->value());
            style->blockSignals(false);
            emit style->StyleChanged(*style);
        }
    }
    void updateChartStyle(ChartStyle* style) {
        if (!isLoading) {
            style->setBackgroundColor(chartbackcolor->getColor());
            style->setAxisColor(chartforecolor->getColor());
            style->setLabelFontSize(chartlabelfontsize->value());
            style->setTickLabelFontSize(charttickfontsize->value());
            style->setFontFamily(chartfont->currentText());
        }
    }

    void updateLineChartStyle(ChartStyle* style) {
        if (!isLoading) {
            style->setTraceLineWidth(linewidth->value());
            style->setTraceFillOpacity(linefill->value());
            style->setTraceFillGradient(linegradient->isChecked());
            style->setGrid(linegrid->isChecked());
            style->setNormalization(static_cast<ROIVert::NORMALIZATION>(linenorm->currentIndex()));

        }
    }
    void updateRidgeChartStyle(ChartStyle* style) {
        if (!isLoading) {
            style->setTraceLineWidth(ridgewidth->value());
            style->setTraceFillOpacity(ridgefill->value());
            style->setTraceFillGradient(ridgegradient->isChecked());
            style->setGrid(ridgegrid->isChecked());
        }
    }


    void loadFromTV() {
        // set gui state from TraceView:
        auto cls{ traceview->getCoreLineChartStyle() };
        auto crs{ traceview->getCoreRidgeChartStyle() };

        // General (comes from line)
        if (cls != nullptr) {
            chartbackcolor->setColor(cls->getBackgroundColor());
            chartforecolor->setColor(cls->getAxisPen().color());
            chartlabelfontsize->setValue(cls->getLabelFont().pointSize());
            charttickfontsize->setValue(cls->getTickLabelFont().pointSize());
            chartfont->setCurrentText(cls->getTickLabelFont().family());

            // Line:
            linewidth->setValue(cls->getTracePen().style() == Qt::NoPen ? 0 : cls->getTracePen().width());
            linefill->setValue(cls->getTraceBrush().style() == Qt::NoBrush ? 0 : cls->getTraceBrush().color().alpha());
            linegradient->setChecked(cls->getTraceFillGradient());
            linegrid->setChecked(cls->getGrid());
            linenorm->setCurrentIndex(static_cast<int>(cls->getNormalization()));
        }

        // Ridge:
        if (crs != nullptr) {
            ridgewidth->setValue(crs->getTracePen().style() == Qt::NoPen ? 0 : crs->getTracePen().width());
            ridgefill->setValue(crs->getTraceBrush().style() == Qt::NoBrush ? 0 : crs->getTraceBrush().color().alpha());
            ridgegradient->setChecked(crs->getTraceFillGradient());
            ridgegrid->setChecked(crs->getGrid());
        }
        ridgeoverlap->setValue(traceview->getRidgeChart().offset * 100);
    }
    void loadFromROIs() {
        auto style = rois->getCoreROIStyle();
        if (style != nullptr) {
            roilinewidth->setValue(style->getPen().style() == Qt::NoPen ? 0 : style->getPen().width());
            roiselsize->setValue(style->getSelectorSize());
            roifillopacity->setValue(style->getBrush().style() == Qt::NoBrush ? 0 : style->getBrush().color().alpha());
        }
        linematchy->setChecked(rois->getMatchYAxes());
    }
    bool isLoading{ false };
};

StyleWidget::StyleWidget(QWidget* parent) : QDockWidget(parent)
{
    this->setWidget(impl->tab);
    impl->doLayout();
    impl->doConnect(this);
}
void StyleWidget::ROIColorChange() {
    if (impl->rois) {
        auto inds = impl->rois->getSelected();
        for (auto& ind : inds) {
            auto thisStyle = impl->rois->getROIStyle(ind);
            thisStyle->setColor(impl->roicolor->getColor());
        }
    }
}
void StyleWidget::ROIStyleChange() {
    impl->updateROIStyle(impl->rois->getCoreROIStyle());
    std::vector<size_t> inds(impl->rois->getNROIs());
    std::iota(inds.begin(), inds.end(), 0);
    for (auto& ind : inds) {
        impl->updateROIStyle(impl->rois->getROIStyle(ind));
    }

}
void StyleWidget::ChartStyleChange() {
    impl->updateChartStyle(impl->traceview->getCoreLineChartStyle());
    impl->updateChartStyle(impl->traceview->getRidgeChart().getStyle());

    impl->traceview->getRidgeChart().updateStyle();

    std::vector<size_t> inds(impl->rois->getNROIs());
    std::iota(inds.begin(), inds.end(), 0);
    for (auto& ind : inds) {
        impl->updateChartStyle(impl->rois->getLineChartStyle(ind));
        impl->rois->updateLineChartStyle(ind);

        impl->updateChartStyle(impl->rois->getRidgeChartStyle(ind));
        impl->rois->updateRidgeChartStyle(ind);

    }
}
void StyleWidget::LineChartStyleChange() {
    impl->updateLineChartStyle(impl->traceview->getCoreLineChartStyle());

    std::vector<size_t> inds(impl->rois->getNROIs());
    std::iota(inds.begin(), inds.end(), 0);
    for (auto& ind : inds) {
        auto style = impl->rois->getLineChartStyle(ind);
        impl->updateLineChartStyle(style);
        impl->rois->updateLineChartStyle(ind);
    }
}
void StyleWidget::RidgeChartStyleChange() {
    impl->updateRidgeChartStyle(impl->traceview->getCoreRidgeChartStyle());

    std::vector<size_t> inds(impl->rois->getNROIs());
    std::iota(inds.begin(), inds.end(), 0);
    for (auto& ind : inds) {
        auto style = impl->rois->getRidgeChartStyle(ind);
        impl->updateRidgeChartStyle(style);
    }
    impl->traceview->getRidgeChart().update();
}
void StyleWidget::RidgeOverlapChange() {
    if (!impl->isLoading) {
        impl->traceview->getRidgeChart().offset = static_cast<float>(impl->ridgeoverlap->value()) / 100.;
        impl->traceview->getRidgeChart().updateOffsets();
    }
}
void StyleWidget::selectionChange(std::vector<size_t> inds) {
    if (inds.empty()) {
        impl->roicolor->setEnabled(false);
    }
    else {
        auto style = impl->rois->getROIStyle(inds.back());
        impl->roicolor->setColor(style->getLineColor());
        impl->roicolor->setEnabled(true);
    }
}
void StyleWidget::setROIs(ROIs* rois) {
    impl->rois = rois;
    connect(rois, &ROIs::selectionChanged, this, &StyleWidget::selectionChange);
}
void StyleWidget::setTraceView(TraceView* traceview) {
    impl->traceview = traceview;
}
void StyleWidget::loadSettings() {
    if (impl->traceview != nullptr && impl->rois != nullptr) {
        impl->isLoading = true;
        impl->loadFromTV();
        impl->loadFromROIs();
        impl->isLoading = false;
    }
}
void StyleWidget::LineMatchyChange() {
    impl->rois->setMatchYAxes(impl->linematchy->isChecked());
}

void StyleWidget::setContentsEnabled(bool onoff) {
    impl->tab->setEnabled(onoff);
}



QSize CustomTabStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
    const QSize& size, const QWidget* widget) const {
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab) {
        s.transpose();
    }
    return s;
}

void CustomTabStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == CE_TabBarTabLabel) {
        if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
            QStyleOptionTab opt(*tab);
            opt.shape = QTabBar::RoundedNorth;
            QProxyStyle::drawControl(element, &opt, painter, widget);
            return;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}
