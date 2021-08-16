#pragma once

#include <QWidget>
#include "widgets/TraceChartImpl/Core.h"

namespace cv {
    class Mat;
}
class QMargins;

class TraceChartWidget : public QWidget
{
    Q_OBJECT

public:
    TraceChartWidget(QWidget* parent = nullptr);
    ~TraceChartWidget();

    void setData(cv::Mat data, 
        QString name, 
        const double& xmin = 0., 
        const double& xmax = 1., 
        const double& offset = 0., 
        TraceChart::NORM norm = TraceChart::NORM::NONE);

    void setData(cv::Mat datas, 
        QStringList names, 
        const std::vector<double>& xmins, 
        const std::vector<double>& xmaxs, 
        const std::vector<double>& offsets, 
        TraceChart::NORM norm = TraceChart::NORM::NONE);

    void removeData(const QString& name);
    void removeData(const QStringList& names);

    void clearData();

    void setStyle(TraceChart::ChartStyle);
    TraceChart::ChartStyle getStyle();

    void setInnerMargins(QMargins marg);
    QMargins getInnerMargins();

    void setTitle(const QString& title) noexcept;
    QString getTitle() const noexcept;

    void setXLabel(const QString& title);
    QString getXLabel() const noexcept;

    void setYLabel(const QString& title);
    QString getYLabel() const noexcept;

    QSize minimumSizeHint() const override;

    void setAntiAliasing(bool) noexcept;
    bool getAntiAliasing() const noexcept;

    void saveAsImage(const QString& filename, int outputwidth = -1, int outputheight = -1, int quality = -1);

signals:
    void clicked(QPointF datalocation, int seriesindex);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

