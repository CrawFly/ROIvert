#pragma once
#include "DockWidgetWithSettings.h"
#include <QProxyStyle>
#include <QStyleOption>

class ROIs;
class TraceViewWidget;

class StyleWidget : public DockWidgetWithSettings
{
    Q_OBJECT
public:
    explicit StyleWidget(QWidget* parent = nullptr);
    ~StyleWidget();

    void setROIs(ROIs* rois);
    void setTraceView(TraceViewWidget* traceview);
    void loadSettings();
    void setContentsEnabled(bool);

    void saveSettings(QSettings& settings) const override;
    void restoreSettings(QSettings& settings) override;
    void resetSettings() override;
public slots:
    void selectionChange(std::vector<size_t> inds);

    void ROIColorChange();
    void ROIStyleChange();
    void ChartStyleChange();
    void LineChartStyleChange();
    void RidgeChartStyleChange();
    void RidgeOverlapChange();
    void LineMatchyChange();

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};

class CustomTabStyle : public QProxyStyle {
public:
    QSize sizeFromContents(ContentsType, const QStyleOption*, const QSize&, const QWidget*) const;
    void drawControl(ControlElement, const QStyleOption*, QPainter*, const QWidget*) const;
};
