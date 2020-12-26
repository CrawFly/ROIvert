/**
 * \class  ContrastPickWidget.h
 *
 * \brief  Widget for selecting min, max, gamma
 *
 * \author neuroph
*/
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

using namespace QtCharts;
/**
 * @brief CPWImpl contains private implementation details for ContrastPickWidget
*/
namespace CPWImpl {
    class VertLine : public QObject, public QGraphicsLineItem
    {
        Q_OBJECT

    public:
        VertLine(QChart* chart);

        void updatePos();
        void setX(qreal x);
        const qreal getX();
        void setMin(qreal m);
        void setMax(qreal m);

    signals:
        void changeVal(qreal val);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    private:
        QChart* ch;
        qreal X = 0.;
        qreal minX = 0.;
        qreal maxX = 1.;
        qreal minY = 0.;
        qreal maxY = 1.;

        QPointF chartToScene(QPointF chartpos) const;
        QPointF sceneToChart(QPointF scenepos) const;
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
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    private:
        QChart* ch;
        qreal gamma = 0.;
        qreal minX = 0.;
        qreal maxX = 1.;
        qreal minY = 0.;
        qreal maxY = 1.;
        qreal mingamma = .001;
        qreal maxgamma = 10;

        QPointF chartToScene(QPointF chartpos) const;
        QPointF sceneToChart(QPointF scenepos) const;
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
        void resizeEvent(QResizeEvent* event) override;

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
}

class ContrastPickWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContrastPickWidget(QWidget* parent = nullptr);
    ~ContrastPickWidget();

    /**
     * @brief Get the current minimum value
     * @return The minimum value
    */
    const double getMin();

    /**
     * @brief Get the current maximum value
     * @return The maximum value
    */
    const double getMax();


    /**
     * @brief Get the current gamma value
     * @return The gamma value
    */
    const double getGamma();


    /**
     * @brief Set the widget's histogram
     * @param histval vector containing histogram values 
    */
    void setHist(std::vector<float> histval);

    /**
     * @brief set all three contrast values
     * @param min Minimum
     * @param max Maximum
     * @param gamma Gamma
    */
    void setContrast(float min, float max, float gamma);

signals:
    /**
     * @brief Signal that fires when contrast changes
    */
    void contrastChanged(double min, double max, double gamma);

private:
    QDoubleSpinBox* spinMin = new QDoubleSpinBox;
    QDoubleSpinBox* spinMax = new QDoubleSpinBox;
    QDoubleSpinBox* spinGamma = new QDoubleSpinBox;
    CPWImpl::ContrastHistogramChart* contChart;
};

