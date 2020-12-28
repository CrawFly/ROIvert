#pragma once
#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QDoubleSpinBox>
#include <QWidget>

typedef std::tuple<float, float, float> contrast;

namespace impl {
    class VertLine : public QObject, public QGraphicsLineItem
    {
        Q_OBJECT

    public:
        explicit VertLine(QGraphicsScene* scene = nullptr);
        void setX(qreal x, bool silent = false);
        void setMin(qreal m);
        void setMax(qreal m);
        void setColor(QColor clr, qreal linewidth = 3.);
        qreal getX() { return X; };

    signals:
        void changeVal(qreal val);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        QRectF boundingRect() const override;

    private:
        qreal X = 0;
        qreal minX = 0;
        qreal maxX = 1;
    };
    class GammaLine : public QObject, public QGraphicsPathItem {
        Q_OBJECT

    public:
        explicit GammaLine(QGraphicsScene* scene = nullptr);
        void setMin(qreal m);
        void setMax(qreal m);
        void setMinGamma(qreal m);
        void setMaxGamma(qreal m);
        void setGamma(qreal g, bool silent = false);
        void setColor(QColor clr, qreal lw = 3.);
        qreal getGamma() { return Gamma; }

    signals:
        void changeVal(qreal val);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        QPainterPath shape() const override;
    private:
        void updateLine();
        qreal Gamma = 1., minX = 0., maxX = 1., mingamma = .001, maxgamma = 10.;
    };
    class ContrastChart : public QGraphicsView
    {
        Q_OBJECT

    public:
        ContrastChart(QWidget* parent = nullptr);
        void setHistogram(QVector<float> y);
        void setHistogramColor(QColor clr, qreal linewidth = 2, bool dogradfill = true);
        void setLineColor(QColor clr, qreal linewidth = 3);

        void setValues(contrast minmaxgamma);
        contrast getValues();

        void setGammaRange(qreal mingamma, qreal maxgamma);
    signals:
        void contrastChanged(contrast minmaxgamma);

    protected:
        void resizeEvent(QResizeEvent* event) override;
    private:
        QGraphicsPathItem* HistPath;
        VertLine* minline;
        VertLine* maxline;
        GammaLine* gamline;
    };
}

class ContrastWidget : public QWidget {
    Q_OBJECT
public:
    explicit ContrastWidget(QWidget* parent = nullptr);

    // These are all passthru to chart
    void setContrast(contrast c) { chart->setValues(c); }
    contrast getContrast() { return chart->getValues(); }
    void setHistogram(QVector<float> y) { chart->setHistogram(y); }
    void setHistogramColor(QColor c) { chart->setHistogramColor(c); }
    void setLineColor(QColor c) { chart->setLineColor(c); }
    void setGammaRange(qreal mingamma, qreal maxgamma) { chart->setGammaRange(mingamma, maxgamma); };
signals:
    void contrastChanged(contrast minmaxgamma);

private:
    impl::ContrastChart* chart = new impl::ContrastChart;
    QDoubleSpinBox* spinMin = new QDoubleSpinBox;
    QDoubleSpinBox* spinGamma = new QDoubleSpinBox;
    QDoubleSpinBox* spinMax = new QDoubleSpinBox;
};
