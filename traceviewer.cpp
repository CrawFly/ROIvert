#include "traceviewer.h"
#include "qvalueaxis.h"
#include "qpoint.h"
#include "qgridlayout.h"
#include "qapplication.h"
#include "qscrollbar.h"
#include "qdebug.h"
#include "qabstractseries.h"
#include "qgraphicslayout.h"
using QtCharts::QValueAxis;

TraceViewer::TraceViewer(QWidget* parent)
    : QWidget(parent)
{
    QGridLayout* glay = new QGridLayout;
    this->setLayout(glay);

    lay = new QVBoxLayout(this);
    scrollArea = new QScrollArea(this);
    QWidget* scrollAreaContent = new QWidget;
    scrollAreaContent->setLayout(lay);
    scrollArea->setWidget(scrollAreaContent);
    scrollArea->setWidgetResizable(true);
    lay->setAlignment(Qt::AlignTop);
    lay->setContentsMargins(0, 0, 0, 0);
    scrollArea->setMinimumHeight(302);
    
    glay->addWidget(scrollArea);
}

TraceViewer::~TraceViewer()
{
}
void TraceViewer::setTrace(const int roiid, const std::vector<double> trace) {
    if (roiid < 1 || roiid > charts.size() + 1) {
        return;
    }
    size_t ind = (size_t)roiid - 1;
    if (charts.size() == ind) {
        push_chart(roiid);
    }

    // build trace, will need time here
    QVector<QPointF> pts;
    double coeff = maxtime/trace.size();
    for (size_t i = 0; i < trace.size(); i++) {
        pts.push_back(QPointF(i*coeff, trace[i]));
    }
    
    QLineSeries* series = qobject_cast<QLineSeries*>(charts[ind]->series()[0]);
    series->replace(pts);

    charts[ind]->axes(Qt::Horizontal)[0]->setMax(pts.size() * coeff);
    charts[ind]->axes(Qt::Horizontal)[0]->setMin(0.);
    double ymin = *std::min_element(trace.begin(), trace.end());
    double ymax = *std::max_element(trace.begin(), trace.end());
    charts[ind]->axes(Qt::Vertical)[0]->setMin(ymin);
    charts[ind]->axes(Qt::Vertical)[0]->setMax(ymax);
    
    chartviews[ind]->show();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    scrollArea->verticalScrollBar()->setValue(chartviews[ind]->y() + chartviews[ind]->height());
}

void TraceViewer::setmaxtime(double t_secs) {
    maxtime = t_secs;

    // need to update existing charts
    for (size_t i = 0; i < charts.size(); i++) { // for each chart
        //qobject_cast<QLineSeries*>charts[i]->series()[0];
        QLineSeries* series = qobject_cast<QLineSeries*>(charts[i]->series()[0]);
        QVector<QPointF> data = series->pointsVector();

        double coeff = maxtime / data.size();
        for (int ii = 0; ii < data.size(); ii++) {
            data[ii].setX(ii * coeff);
        }
        series->replace(data);
        charts[i]->axes(Qt::Horizontal)[0]->setMax(data.size() * coeff);
    }
}

void TraceViewer::push_chart(int roiid) {
    QColor foreClr = QColor(Qt::lightGray);

    QChart* chart = new QChart;
    QLineSeries* series = new QLineSeries;
    ChartViewClick* chartView = new ChartViewClick;
    chartView->setChart(chart);
    chartView->setParent(this);

    chart->legend()->hide();
    QString title = "ROI " + QString::number(roiid);
    chart->setTitle(title);
    chart->layout()->setContentsMargins(1, 0, 1, 0);

    chartView->setFixedHeight(300);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setContentsMargins(0, 0, 0, 0);

    series->append(0, 0);
    chart->addSeries(series);
    chart->createDefaultAxes();
    QValueAxis* x = (QValueAxis*)chart->axes(Qt::Horizontal)[0];
    QValueAxis* y = (QValueAxis*)chart->axes(Qt::Vertical)[0];

    x->setTitleText(QString::fromWCharArray(L"\x03B4\xD835\xDC53/\xD835\xDC53"));
    y->setTitleText("Time (s)");
    x->setGridLineColor(QColor(Qt::darkGray));
    y->setGridLineColor(QColor(Qt::darkGray));

    QPen seriespen(selclr, 3);
    series->setPen(seriespen);
    chart->setBackgroundBrush(QBrush("#222222"));

    x->setLabelsColor(foreClr);
    y->setLabelsColor(foreClr);
    x->setTitleBrush(foreClr);
    y->setTitleBrush(foreClr);
    chart->setTitleBrush(foreClr);

    charts.push_back(chart);
    chartviews.push_back(chartView);
    lay->addWidget(chartView);

    connect(chartView, &ChartViewClick::clicked, this, [=]() {emit chartClicked(roiid); });
}

void TraceViewer::setSelectedTrace(int oldind, int newind) {
    if (oldind > 0 && oldind-1 < charts.size()) {
        QLineSeries* series = qobject_cast<QLineSeries*>(charts[oldind-1]->series()[0]);
        series->setColor(unselclr);
    }
    if (newind > 0 && newind-1 < charts.size()) {
        QLineSeries* series = qobject_cast<QLineSeries*>(charts[newind-1]->series()[0]);
        series->setColor(selclr);
        scrollArea->verticalScrollBar()->setValue(chartviews[newind - 1]->y());
    }
}