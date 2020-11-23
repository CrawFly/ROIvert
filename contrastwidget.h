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


// ugh tried to use a nested class approach, defining vertline,gammaline, and contrastHistogramChart in ContrastWidget, but failed...:(
// https://softwareengineering.stackexchange.com/questions/382687/using-friend-classes-to-encapsulate-private-member-functions-in-c-good-pract
// stuffing these in header for now, would like to put them somewhere privater? they're only used by ContrastWidget...


class VertLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT

public:
    VertLine(QChart* chart) {
        ch = chart;
        ch = chart;
        setCursor(Qt::SizeHorCursor);
        QPen pen(QColor("#2274A5"), 3);
        pen.setCosmetic(true);
        setPen(pen);
    };

    void updatePos() {
        // call this on resize
        QPointF topleft = chartToScene(QPointF(X, maxY));
        QPointF bottomright = chartToScene(QPointF(X, minY));
        //setRect(QRectF(topleft, bottomright));
        setLine(topleft.x(), topleft.y(), bottomright.x(), bottomright.y());
    }

    void setX(qreal x) { X = x; updatePos(); emit changeVal(X); }
    qreal getX() { return X; }
    void setMin(qreal m) { minX = m; };
    void setMax(qreal m) { maxX = m; };

signals:
    void changeVal(qreal val); // fires when X changes

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) {
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
        qreal newx = sceneToChart(event->pos()).x();
        newx = qMin(qMax(newx, minX), maxX);
        setX(newx);
    }

private:
    QChart* ch;
    qreal X = 0.;
    qreal minX = 0.;
    qreal maxX = 1.;
    qreal minY = 0.;
    qreal maxY = 1.;

    QPointF chartToScene(QPointF chartpos) {
        return ch->mapToPosition(chartpos);
    }
    QPointF sceneToChart(QPointF scenepos) {
        return ch->mapToValue(scenepos);
    }
};

class GammaLine : public QObject, public QGraphicsPathItem
{
    Q_OBJECT

public:
    GammaLine(QChart* chart) {
        ch = chart;
        setCursor(Qt::SizeVerCursor);
        QPen pen(QColor("#2274A5"), 3);
        pen.setCosmetic(true);
        setPen(pen);
        setBrush(QBrush(Qt::transparent));
    };
    void updateCurve() {
        // Make 100 points between minx and maxx (x_chart)
        //  define: x=i/100; y=(x^gamma - miny) / (maxx - minx);
        // scale y to make y_chart
        float xdelt = maxX - minX;
        float ydelt = maxY - minY;
        QPointF startpoint = chartToScene(QPointF(minX, minY));

        // Doing this as a QPainterPath let's me leave it unclosed (as opposed to a QPolygonShapeItem)
        // But I can't get the mouse to not hit the transparent area that would define the closed poly
        // So I just define points up and back down...
        QPainterPath pth(startpoint);
        for (int i = 0; i <= 100; i++) {
            float x = i / 100.;  // this is x for the gamma equation
            QPointF p(x * xdelt + minX, (pow(x, gamma) - minY) / ydelt);
            pth.lineTo(chartToScene(p));
        }
        for (int i = 100; i >= 0; i--) {
            float x = i / 100.;
            QPointF p(x * xdelt + minX, (pow(x, gamma) - minY) / ydelt);
            pth.lineTo(chartToScene(p));
        }
        setPath(pth);
    }

    void setMin(qreal m) { minX = m; updateCurve(); }
    void setMax(qreal m) { maxX = m; updateCurve(); }
    void setMinGamma(qreal m) { mingamma = m; }
    void setMaxGamma(qreal m) { maxgamma = m; }
    void setGamma(qreal g) { gamma = qMin(qMax(g, mingamma), maxgamma); updateCurve(); emit changeVal(gamma); }

signals:
    void changeVal(qreal val);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) {}
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
        QPointF newloc = sceneToChart(event->pos());

        // Cap it
        newloc.setX(qMin(qMax(newloc.x(), minX), maxX));
        newloc.setY(qMin(qMax(newloc.y(), minY), maxY));

        // Convert the x to be the distnace between min and max
        newloc.setX((newloc.x() - minX) / (maxX - minX));

        // Now Solve for gamma
        qreal g = log(newloc.y()) / log(newloc.x());

        // Now update:
        setGamma(g);
    }

private:
    QChart* ch;
    qreal gamma = 0.; //
    qreal minX = 0.;
    qreal maxX = 1.;
    qreal minY = 0.;
    qreal maxY = 1.;
    qreal mingamma = .001;
    qreal maxgamma = 10;

    QPointF chartToScene(QPointF chartpos) {
        return ch->mapToPosition(chartpos);
    }
    QPointF sceneToChart(QPointF scenepos) {
        return ch->mapToValue(scenepos);
    }
};

class ContrastHistogramChart : public QGraphicsView {
    Q_OBJECT

public:
    ContrastHistogramChart(QWidget* parent = 0)
        : QGraphicsView(new QGraphicsScene, parent),
        chart(0)
    {
        // View Setup
        setDragMode(QGraphicsView::NoDrag);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setRenderHint(QPainter::Antialiasing);

        // Chart Setup 
        chart = new QChart;
        chart->legend()->hide();
        chart->setMargins(QMargins(0, 0, 0, 0));
        chart->setBackgroundRoundness(0.);
        chart->layout()->setContentsMargins(1, 0, 1, 0);
        scene()->addItem(chart);

        // Histogram line setup:
        // Note: attempt 1 used an area, it looked nice but was insanely slow!
        inLineSeries = new QLineSeries;
        outLineSeries = new QLineSeries;
        chart->addSeries(outLineSeries);
        chart->addSeries(inLineSeries);

        
        QPen inpen(QColor("#F75C03"), 3);
        inpen.setCosmetic(true);
        inLineSeries->setPen(inpen);
        QPen outpen(QColor("#F75C03"), 1);
        outpen.setCosmetic(true);
        outLineSeries->setPen(outpen);

        // min/max/gamma lines:
        minline = new VertLine(chart);
        minline->setMin(0.); minline->setMax(1.); minline->setX(0.);
        this->scene()->addItem(minline);

        maxline = new VertLine(chart);
        maxline->setMin(0.); maxline->setMax(1.); maxline->setX(1.);
        this->scene()->addItem(maxline);

        gamline = new GammaLine(chart);
        gamline->setMin(0.); gamline->setMax(1.); gamline->setGamma(1.);
        this->scene()->addItem(gamline);

        connect(minline, &VertLine::changeVal, this, &ContrastHistogramChart::updateMin);
        connect(maxline, &VertLine::changeVal, this, &ContrastHistogramChart::updateMax);
        connect(gamline, &GammaLine::changeVal, this, &ContrastHistogramChart::gammaChanged);

    }
    void setData(const std::vector<float>& data) {
        inLineSeries->clear();
        outLineSeries->clear();

        QVector<QPointF> newdata(data.size());
        QVector<QPointF> newdatain(data.size());

        float xmax = (float(data.size()) - 1);
        float ymax = -1;
        for (size_t i = 0; i < data.size(); i++) {
            ymax = std::max(ymax, data[i]);
        }

        for (size_t i = 0; i < data.size(); i++) {
            newdata.push_back(QPointF(i / xmax, data[i] / ymax));
        }
        outLineSeries->replace(newdata);
        updateInLine();
    }
    void setGammaRange(float mingamma, float maxgamma) {
        gamline->setMinGamma(mingamma);
        gamline->setMaxGamma(maxgamma);

    }

signals:
    void minChanged(double newmin);
    void maxChanged(double newmin);
    void gammaChanged(double newmin);

public slots:
    void changeMin(double newval) { minline->setX(newval); }
    void changeMax(double newval) { maxline->setX(newval); }
    void changeGamma(double newval) { gamline->setGamma(newval); }

protected:
    void resizeEvent(QResizeEvent* event) {
        this->setMaximumHeight(this->width() / 2.);
        if (scene()) {
            chart->resize(event->size());
            scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
            minline->updatePos();
            maxline->updatePos();
            gamline->updateCurve();
        }
        QGraphicsView::resizeEvent(event);
    }

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

    void updateMin(qreal val) {
        // called whenever the minline value changes
        maxline->setMin(val);
        gamline->setMin(val);
        emit minChanged((double)val);
        updateInLine();
    }
    void updateMax(qreal val) {
        // called whenever the maxline value changes...
        minline->setMax(val);
        gamline->setMax(val);
        emit maxChanged((double)val);
        updateInLine();
    }
    void updateInLine() {
        // just take outline, find data that are in, update inline
        QVector<QPointF> data(outLineSeries->pointsVector());
        QVector<QPointF> indata;
        for (int i = 0; i < data.size(); i++) {
            if (data[i].x() >= minline->getX() && data[i].x() <= maxline->getX()) {
                indata.push_back(data[i]);
            }
        }
        inLineSeries->replace(indata);
    }
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

signals:
    void contrastChanged(double min, double max, double gamma);

private:
    QDoubleSpinBox* spinMin = new QDoubleSpinBox;
    QDoubleSpinBox* spinMax = new QDoubleSpinBox;
    QDoubleSpinBox* spinGamma = new QDoubleSpinBox;
    ContrastHistogramChart* contChart;
};

