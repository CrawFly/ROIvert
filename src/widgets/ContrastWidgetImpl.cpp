#include "widgets/ContrastWidgetImpl.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>

VertLine::VertLine(QGraphicsScene* scene)
{
    if (scene)
    {
        scene->addItem(this);
    }
    setCursor(Qt::SizeHorCursor);
    setColor(QColor("#2274A5"));
    setX(X, true);
}
void VertLine::setColor(QColor clr, qreal lw)
{
    QPen pen(clr, lw);
    pen.setCosmetic(true);
    setPen(pen);
}
void VertLine::setX(qreal x, bool silent)
{
    if (x >= minX && x <= maxX)
    {
        X = x;
        setLine(X, 0, X, 1);
        if (!silent)
        {
            emit changeVal(X);
        }
    }
}
void VertLine::setMin(qreal m) { minX = m; }
void VertLine::setMax(qreal m) { maxX = m; }
void VertLine::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}
void VertLine::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    setX(event->pos().x());
}
QRectF VertLine::boundingRect() const
{
    return QRectF(X - .025, 0, .05, 1);
};

GammaLine::GammaLine(QGraphicsScene* scene)
{
    if (scene)
    {
        scene->addItem(this);
    }
    setCursor(Qt::SizeVerCursor);
    setColor(QColor("#2274A5"));
    setGamma(Gamma);
}
void GammaLine::setColor(QColor clr, qreal lw)
{
    QPen pen(clr, lw);
    pen.setCosmetic(true);
    setPen(pen);
}
void GammaLine::setMin(qreal m)
{
    minX = m;
    updateLine();
}
void GammaLine::setMax(qreal m)
{
    maxX = m;
    updateLine();
}
void GammaLine::setMinGamma(qreal m) { mingamma = m; }
void GammaLine::setMaxGamma(qreal m) { maxgamma = m; }
void GammaLine::setGamma(qreal g, bool silent)
{
    if (g >= mingamma && g <= maxgamma)
    {
        Gamma = g;
        updateLine();
        if (!silent)
        {
            emit changeVal(Gamma);
        }
    }
}
void GammaLine::mousePressEvent(QGraphicsSceneMouseEvent* event) {}
void GammaLine::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF newloc = event->pos();

    // Cap x/y to minmax
    newloc.setX(qMin(qMax(newloc.x(), minX), maxX));
    newloc.setY(1 - qMin(qMax(newloc.y(), 0.), 1.));

    // Convert x to be the distance between min and max
    newloc.setX((newloc.x() - minX) / (maxX - minX));

    // Solve for gamma and update:
    setGamma(log(newloc.y()) / log(newloc.x()));
}
void GammaLine::updateLine()
{
    float xdelt = maxX - minX;
    QPointF startpoint(minX, 1);

    QPainterPath pth(startpoint);

    for (int i = 0; i <= 100; ++i)
    {
        float x = i / 100.; // this is x for the gamma equation
        QPointF p(x * xdelt + minX, 1 - (pow(x, Gamma)));
        pth.lineTo(p);
    }

    setPath(pth);
}
QPainterPath GammaLine::shape() const
{
    QPainterPath pth, rpth;
    pth = path();
    rpth = path().toReversed();

    pth.translate(0, .025);
    rpth.translate(0, -.025);

    pth.connectPath(rpth);
    return pth;
}

namespace ContrastWidgetImpl
{
    ContrastChart::ContrastChart(QWidget* parent)
        : QGraphicsView(parent)
    {
        
        auto bg = palette().color(QPalette::Active, QPalette::Window);
        
        QGraphicsScene* scene = new QGraphicsScene(0, 0, 1, 1);
        setRenderHints(QPainter::Antialiasing);

        scene->setBackgroundBrush(QColor(bg));

        setScene(scene);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // Histogram
        HistPath = new QGraphicsPathItem;
        setHistogramColor(QColor(255-bg.red(),255-bg.green(),255-bg.blue()), 2);
        scene->addItem(HistPath);

        // Overlays to mark out of min/max
        QGraphicsRectItem* leftOverlayRect = new QGraphicsRectItem;
        leftOverlayRect->setRect(0, -1, 0, 3);
        leftOverlayRect->setPen(Qt::NoPen);
        leftOverlayRect->setBrush(QBrush(QColor(bg.red(), bg.green(), bg.blue(), 200)));
        scene->addItem(leftOverlayRect);
        QGraphicsRectItem* rightOverlayRect = new QGraphicsRectItem;
        rightOverlayRect->setRect(1, -1, 0, 3);
        rightOverlayRect->setPen(QPen(Qt::NoPen));
        rightOverlayRect->setBrush(QBrush(QColor(bg.red(), bg.green(), bg.blue(), 200)));
        scene->addItem(rightOverlayRect);

        // min/max lines
        minline = new VertLine(scene);
        minline->setX(0.);
        minline->setMin(0.);
        minline->setMax(1.);
        minline->setObjectName("minline");

        maxline = new VertLine(scene);
        maxline->setX(1.);
        maxline->setMin(0.);
        maxline->setMax(1.);
        maxline->setObjectName("maxline");

        // gamma curve
        gamline = new GammaLine(scene);
        gamline->setObjectName("gamline");

        // connect connect min/max/gamma with eachother
        connect(minline, &VertLine::changeVal, maxline, &VertLine::setMin);
        connect(maxline, &VertLine::changeVal, minline, &VertLine::setMax);
        connect(minline, &VertLine::changeVal, gamline, &GammaLine::setMin);
        connect(maxline, &VertLine::changeVal, gamline, &GammaLine::setMax);

        // connect overlays to accompanying lines
        connect(minline, &VertLine::changeVal, this, [leftOverlayRect](qreal val)
        { leftOverlayRect->setRect(-1, -1, 1 + val, 3); });
        connect(maxline, &VertLine::changeVal, this, [rightOverlayRect](qreal val)
        { rightOverlayRect->setRect(val, -1, 2. - val, 3); });

        // connect outbound signal:
        auto lam = [this] {
            emit contrastChanged(getValues());
        };
        connect(minline, &VertLine::changeVal, this, lam);
        connect(maxline, &VertLine::changeVal, this, lam);
        connect(gamline, &GammaLine::changeVal, this, lam);
    }
    void ContrastChart::setHistogram(QVector<float> y)
    {
        QPainterPath pth;
        const float ymax = *std::max_element(y.constBegin(), y.constEnd());

        pth.moveTo(0, 1);
        for (int i = 0; i < y.size(); i++)
        {
            pth.lineTo((float)i / y.size(), 1. - y[i] / ymax);
        }
        pth.lineTo(1, 1);
        HistPath->setPath(pth);
    }
    void ContrastChart::setHistogramColor(QColor clr, qreal linewidth, bool dogradfill)
    {
        auto bg = palette().color(QPalette::Active, QPalette::Window);
        
        QPen pen(clr, linewidth);
        pen.setCosmetic(true);
        HistPath->setPen(pen);
        if (dogradfill)
        {
            QLinearGradient lGrad(.5, 1, .5, 0);
            lGrad.setColorAt(0, bg);
            clr.setAlpha(20);
            lGrad.setColorAt(.5, clr);
            clr.setAlpha(70);
            lGrad.setColorAt(1, clr);
            HistPath->setBrush(lGrad);
        }
    }
    void ContrastChart::resizeEvent(QResizeEvent* event)
    {
        fitInView(scene()->sceneRect(), Qt::IgnoreAspectRatio);
    }
    void ContrastChart::setLineColor(QColor clr, qreal linewidth)
    {
        minline->setColor(clr, linewidth);
        maxline->setColor(clr, linewidth);
        gamline->setColor(clr, linewidth);
    }
    void ContrastChart::setValues(ROIVert::contrast minmaxgamma, bool silent)
    {
        bool oldblock = signalsBlocked();
        if (silent) {
            blockSignals(true);
        }

        minline->setMax(std::get<1>(minmaxgamma));
        maxline->setMin(std::get<0>(minmaxgamma));

        minline->setX(std::get<0>(minmaxgamma), false);
        maxline->setX(std::get<1>(minmaxgamma), false);
        gamline->setGamma(std::get<2>(minmaxgamma), false);

        if (silent) {
            blockSignals(oldblock);
        }
    }
    void ContrastChart::setGammaRange(qreal mingamma, qreal maxgamma)
    {
        gamline->setMinGamma(mingamma);
        gamline->setMaxGamma(maxgamma);
    }
    ROIVert::contrast ContrastChart::getValues()
    {
        return std::make_tuple(minline->getX(), maxline->getX(), gamline->getGamma());
    }
}
