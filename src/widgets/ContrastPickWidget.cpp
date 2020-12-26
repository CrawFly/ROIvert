#include "ContrastPickWidget.h"
#include "qdebug.h"
#include <QLabel>

namespace CPWImpl {
    // VertLine
    VertLine::VertLine(QChart* chart) {
        ch = chart;
        setCursor(Qt::SizeHorCursor);
        QPen pen(QColor("#2274A5"), 3);
        pen.setCosmetic(true);
        setPen(pen);
    };
    void VertLine::updatePos() {
        // call this on resize
        const QPointF topleft = chartToScene(QPointF(X, maxY));
        const QPointF bottomright = chartToScene(QPointF(X, minY));
        //setRect(QRectF(topleft, bottomright));
        setLine(topleft.x(), topleft.y(), bottomright.x(), bottomright.y());
    }
    void VertLine::setX(qreal x) { X = x; updatePos(); emit changeVal(X); }
    const qreal VertLine::getX() { return X; }
    void VertLine::setMin(qreal m) { minX = m; };
    void VertLine::setMax(qreal m) { maxX = m; };
    void VertLine::mousePressEvent(QGraphicsSceneMouseEvent* event) {}
    void VertLine::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
        qreal newx = sceneToChart(event->pos()).x();
        newx = qMin(qMax(newx, minX), maxX);
        setX(newx);
    }
    QPointF VertLine::chartToScene(QPointF chartpos) const { return ch->mapToPosition(chartpos); }
    QPointF VertLine::sceneToChart(QPointF scenepos) const { return ch->mapToValue(scenepos); }

    // GammaLine
    GammaLine::GammaLine(QChart* chart) {
        ch = chart;
        setCursor(Qt::SizeVerCursor);
        QPen pen(QColor("#2274A5"), 3);
        pen.setCosmetic(true);
        setPen(pen);
        setBrush(QBrush(Qt::transparent));
    };
    void GammaLine::updateCurve() {
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
        for (int i = 0; i <= 100; ++i) {
            float x = i / 100.;  // this is x for the gamma equation
            QPointF p(x * xdelt + minX, (pow(x, gamma) - minY) / ydelt);
            pth.lineTo(chartToScene(p));
        }
        for (int i = 100; i >= 0; --i) {
            float x = i / 100.;
            QPointF p(x * xdelt + minX, (pow(x, gamma) - minY) / ydelt);
            pth.lineTo(chartToScene(p));
        }
        setPath(pth);
    }
    void GammaLine::setMin(qreal m) { minX = m; updateCurve(); }
    void GammaLine::setMax(qreal m) { maxX = m; updateCurve(); }
    void GammaLine::setMinGamma(qreal m) { mingamma = m; }
    void GammaLine::setMaxGamma(qreal m) { maxgamma = m; }
    void GammaLine::setGamma(qreal g) { gamma = qMin(qMax(g, mingamma), maxgamma); updateCurve(); emit changeVal(gamma); }
    void GammaLine::mousePressEvent(QGraphicsSceneMouseEvent* event) {}
    void GammaLine::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
        QPointF newloc = sceneToChart(event->pos());

        // Cap it
        newloc.setX(qMin(qMax(newloc.x(), minX), maxX));
        newloc.setY(qMin(qMax(newloc.y(), minY), maxY));

        // Convert the x to be the distnace between min and max
        newloc.setX((newloc.x() - minX) / (maxX - minX));

        // Now Solve for gamma and update:
        setGamma(log(newloc.y()) / log(newloc.x()));
    }
    QPointF GammaLine::chartToScene(QPointF chartpos) const {
        return ch->mapToPosition(chartpos);
    }
    QPointF GammaLine::sceneToChart(QPointF scenepos) const {
        return ch->mapToValue(scenepos);
    }


    // ContrastHistogramChart
    ContrastHistogramChart::ContrastHistogramChart(QWidget* parent) :
        QGraphicsView(new QGraphicsScene, parent),
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
    void ContrastHistogramChart::setData(const std::vector<float>& data) {
        inLineSeries->clear();
        outLineSeries->clear();

        QVector<QPointF> newdata(data.size());
        QVector<QPointF> newdatain(data.size());

        const float xmax = static_cast<float>(data.size()) - 1;
        float ymax = -1;
        for each (float datum in data)
        {
            ymax = std::max(ymax, datum);
        }

        for (size_t i = 0; i < data.size(); i++) {
            newdata.push_back(QPointF(i / xmax, data[i] / ymax));
        }
        outLineSeries->replace(newdata);
        updateInLine();
    }
    void ContrastHistogramChart::setGammaRange(float mingamma, float maxgamma) {
        gamline->setMinGamma(mingamma);
        gamline->setMaxGamma(maxgamma);

    }
    void ContrastHistogramChart::changeMin(double newval) { minline->setX(newval); }
    void ContrastHistogramChart::changeMax(double newval) { maxline->setX(newval); }
    void ContrastHistogramChart::changeGamma(double newval) { gamline->setGamma(newval); }
    void ContrastHistogramChart::resizeEvent(QResizeEvent* event) {
        //this->setMaximumHeight(this->width());
        if (scene()) {
            chart->resize(event->size());
            scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
            minline->updatePos();
            maxline->updatePos();
            gamline->updateCurve();
        }
        QGraphicsView::resizeEvent(event);
    }
    void ContrastHistogramChart::updateMin(qreal val) {
        // called whenever the minline value changes
        maxline->setMin(val);
        gamline->setMin(val);

        emit minChanged(static_cast<double>(val));
        updateInLine();
    }
    void ContrastHistogramChart::updateMax(qreal val) {
        // called whenever the maxline value changes...
        minline->setMax(val);
        gamline->setMax(val);
        emit maxChanged(static_cast<double>(val));
        updateInLine();
    }
    void ContrastHistogramChart::updateInLine() {
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
}



// ContrastPickWidget
ContrastPickWidget::ContrastPickWidget(QWidget* parent) {
        setParent(parent);

        QVBoxLayout* layV = new QVBoxLayout;
        contChart = new CPWImpl::ContrastHistogramChart;
        layV->addWidget(contChart);

        QHBoxLayout* layH = new QHBoxLayout;
        spinMin->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        spinGamma->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        spinMax->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

        layH->addWidget(spinMin, 0, Qt::AlignLeft);
        layH->addWidget(spinGamma, 0, Qt::AlignHCenter);
        layH->addWidget(spinMax, 0, Qt::AlignRight);

        spinMin->setMinimum(0.);
        spinMin->setMaximum(1.);
        spinMin->setValue(0.);
        spinMin->setSingleStep(.01);

        spinGamma->setMinimum(0.001);
        spinGamma->setMaximum(10.);
        spinGamma->setValue(1.);
        spinGamma->setSingleStep(.01);
        contChart->setGammaRange(.001, 10.);

        spinMax->setMinimum(0.);
        spinMax->setMaximum(1.);
        spinMax->setValue(1.);
        spinMax->setSingleStep(.01);

        layV->addLayout(layH);
        setLayout(layV);
        layV->setAlignment(Qt::AlignTop);

        connect(contChart, &CPWImpl::ContrastHistogramChart::minChanged, this, [=](double val) {spinMin->setValue(val); });
        connect(contChart, &CPWImpl::ContrastHistogramChart::maxChanged, this, [=](double val) {spinMax->setValue(val); });
        connect(contChart, &CPWImpl::ContrastHistogramChart::gammaChanged, this, [=](double val) {spinGamma->setValue(val); });

        connect(spinMin, SIGNAL(valueChanged(double)), contChart, SLOT(changeMin(double)));
        connect(spinMax, SIGNAL(valueChanged(double)), contChart, SLOT(changeMax(double)));
        connect(spinGamma, SIGNAL(valueChanged(double)), contChart, SLOT(changeGamma(double)));

        connect(spinMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { emit contrastChanged(getMin(), getMax(), getGamma());});
        connect(spinMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { emit contrastChanged(getMin(), getMax(), getGamma());});
        connect(spinGamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { emit contrastChanged(getMin(), getMax(), getGamma());});

    }
ContrastPickWidget::~ContrastPickWidget() {};
const double ContrastPickWidget::getMin() { return spinMin->value(); }
const double ContrastPickWidget::getMax() { return spinMax->value(); }
const double ContrastPickWidget::getGamma() { return spinGamma->value(); }
void ContrastPickWidget::setHist(std::vector<float> histval) {contChart->setData(histval);}
void ContrastPickWidget::setContrast(float min, float max, float gamma) {
    spinMin->setValue(min);
    spinMax->setValue(max);
    spinGamma->setValue(gamma);
}
