#pragma once
#include <QWidget>
#include <QDoubleSpinBox>
#include <QGraphicsLineItem>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChart>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QLineSeries>
#include <QGraphicsLayout>
#include <QBoxLayout>
#include "opencv2/opencv.hpp"

using namespace QtCharts;

class VertLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT

public:
    VertLine(QChart* chart);

    void updatePos();
    void setX(qreal x);
    qreal getX();
    void setMin(qreal m);
    void setMax(qreal m);

signals:
    void changeVal(qreal val); 

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
    QChart* ch;
    qreal X = 0.;
    qreal minX = 0.;
    qreal maxX = 1.;
    qreal minY = 0.;
    qreal maxY = 1.;

    QPointF chartToScene(QPointF chartpos);
    QPointF sceneToChart(QPointF scenepos);
};

class GammaLine : public QObject, public QGraphicsPathItem
{
    Q_OBJECT

public:
    GammaLine(QChart* chart);
    void updateCurve();

    void setMin(qreal m);
    void setMax(qreal m);
    void setMinGamma(qreal m);
    void setMaxGamma(qreal m);
    void setGamma(qreal g);

signals:
    void changeVal(qreal val);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
    QChart* ch;
    qreal gamma = 0.;
    qreal minX = 0.;
    qreal maxX = 1.;
    qreal minY = 0.;
    qreal maxY = 1.;
    qreal mingamma = .001;
    qreal maxgamma = 10;

    QPointF chartToScene(QPointF chartpos);
    QPointF sceneToChart(QPointF scenepos);
};

class ContrastHistogramChart : public QGraphicsView {
    Q_OBJECT

public:
    ContrastHistogramChart(QWidget* parent = 0);
    void setData(const std::vector<float>& data);
    void setGammaRange(float mingamma, float maxgamma);

signals:
    void minChanged(double newmin);
    void maxChanged(double newmin);
    void gammaChanged(double newmin);

public slots:
    void changeMin(double newval);
    void changeMax(double newval);
    void changeGamma(double newval);

protected:
    void resizeEvent(QResizeEvent* event);

private:
    QChart* chart;
    VertLine* minline;
    VertLine* maxline;
    GammaLine* gamline;

    QLineSeries* inLineSeries;
    QLineSeries* outLineSeries;

    float minval = 0.;
    float maxval = 1.;
    float gamval = 1.;

    void updateMin(qreal val);
    void updateMax(qreal val);
    void updateInLine();
};

class ContrastWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContrastWidget(QWidget* parent = nullptr);
    ~ContrastWidget();

    double getMin();
    double getMax();
    double getGamma();

    void setHist(std::vector<float> histval);
    void setContrast(float min, float max, float gamma);

signals:
    void contrastChanged(double min, double max, double gamma);

private:
    QDoubleSpinBox* spinMin = new QDoubleSpinBox;
    QDoubleSpinBox* spinMax = new QDoubleSpinBox;
    QDoubleSpinBox* spinGamma = new QDoubleSpinBox;
    ContrastHistogramChart* contChart;
};

