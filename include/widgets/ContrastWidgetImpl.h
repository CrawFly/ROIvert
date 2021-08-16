/**
 * \class  ContrastWidgetImpl.h
 *
 * \brief  Implementation classes used by ContrastWidget
 *
 * \author neuroph
*/
#pragma once
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include "roivertcore.h"

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

namespace ContrastWidgetImpl {
     class ContrastChart : public QGraphicsView
    {
        Q_OBJECT

    public:
        ContrastChart(QWidget* parent = nullptr);
        void setHistogram(QVector<float> y);
        void setHistogramColor(QColor clr, qreal linewidth = 2, bool dogradfill = true);
        void setLineColor(QColor clr, qreal linewidth = 3);

        void setValues(ROIVert::contrast minmaxgamma);
        ROIVert::contrast getValues();

        void setGammaRange(qreal mingamma, qreal maxgamma);
    signals:
        void contrastChanged(ROIVert::contrast minmaxgamma);

    protected:
        void resizeEvent(QResizeEvent* event) override;
    private:
        QGraphicsPathItem* HistPath;
        VertLine* minline;
        VertLine* maxline;
        GammaLine* gamline;
    };
}