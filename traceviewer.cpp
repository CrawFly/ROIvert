#include "traceviewer.h"
#include "qvalueaxis.h"
#include "qpoint.h"
#include "qgridlayout.h"

using QtCharts::QValueAxis;

TraceViewer::TraceViewer(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *glay = new QGridLayout;
    this->setLayout(glay);

    lay = new QVBoxLayout(this);
    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget* scrollAreaContent = new QWidget;
    scrollAreaContent->setLayout(lay);
    scrollArea->setWidget(scrollAreaContent);
    scrollArea->setWidgetResizable(true);
    lay->setAlignment(Qt::AlignTop);
    lay->setContentsMargins(0, 0, 0, 0);
    scrollArea->setMinimumHeight(300);


    glay->addWidget(scrollArea); 
}

TraceViewer::~TraceViewer()
{
}
void TraceViewer::tracecomputed(const int roiid, const std::vector<double> trace) {
    if (roiid < 1 || roiid > charts.size()+1){
        return;
    }

    size_t ind = (size_t)roiid - 1;
    if (charts.size() == ind) {
        QChart* chart = new QChart;
        QLineSeries* series = new QLineSeries;
        QChartView* chartView = new QChartView(chart, this);

        chart->legend()->hide();
        //chart->addSeries(series);
        //QString title = "ROI " + QString::number(ind + 1);
        //chart->setTitle(title);

        //QValueAxis* x = (QValueAxis*)chart->axes(Qt::Horizontal)[0];
        //x->setTitleText("Frame"); // Todo: come back to setting this as time!
        //chart->axes(Qt::Vertical)[0]->setTitleText(QString::fromWCharArray(L"\x03B4\xD835\xDC53/\xD835\xDC53"));

        chartView->setFixedHeight(300);
        chartView->setRenderHint(QPainter::Antialiasing);

        charts.push_back(chart);
        chartviews.push_back(chartView);
        //serieses.push_back(series);

        lay->addWidget(chartView);
    }


    // build trace, will need time here
    QVector<QPointF> pts;
    for (size_t i = 0; i < trace.size(); i++) {
        pts.push_back(QPointF(i, trace[i]));
    }
    QLineSeries* series = new QLineSeries;
    series->replace(pts);
    charts[ind]->removeAllSeries();
    charts[ind]->addSeries(series);
    charts[ind]->createDefaultAxes();
}
void TraceViewer::setmaxtime(float t_msecs) {

}