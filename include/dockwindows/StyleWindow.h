#pragma once
#include <QWidget>
#include <QProxyStyle>
#include <QStyleOption>

class ROIs;
class TraceView;

class StyleWindow : public QWidget
{
    Q_OBJECT
    public:
        explicit StyleWindow(QWidget* parent);
        void setROIs(ROIs* rois);
        void setTraceView(TraceView* traceview);
        void loadSettings();

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
        std::unique_ptr<pimpl>impl = std::make_unique<pimpl>();
};

class CustomTabStyle : public QProxyStyle {
public:
  QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                         const QSize& size, const QWidget* widget) const {
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab) {
      s.transpose();
    }
    return s;
  }

  void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
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
};
