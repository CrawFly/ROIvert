#include <QtTest/QtTest>
#include <QDoubleSpinBox>
#include "tContrastWidget.h"
#include "widgets/ContrastWidget.h"
#include "roivertcore.h"
#include "widgets/ContrastWidgetImpl.h"

constexpr double EPS = .0000001;

template<class Ta, class Tb> bool almostequal(Ta a, Tb b) {
    return abs(a - b) < EPS;
}

struct tContrastWidget::implptrs {
    // these impl tests are necessary to confirm behaviors on this old and poorly written widget.
    implptrs(ContrastWidget* widget) {
        updateptrs(widget);
    }

    void updateptrs(ContrastWidget* widget) {
        spinmin = widget->findChild<QDoubleSpinBox*>("spinMin");
        spinmax = widget->findChild<QDoubleSpinBox*>("spinMax");
        spingamma = widget->findChild<QDoubleSpinBox*>("spinGamma");
        chart = widget->findChild<ContrastWidgetImpl::ContrastChart*>();
    
        auto scenekids = chart->scene()->items();
        for (auto& kid : scenekids) {
            auto v = qgraphicsitem_cast<VertLine*>(kid);
            if (v && v->objectName()=="minline") {
                minline = v;
            }
            else if (v && v->objectName() == "maxline") {
                maxline = v;
            }
            /*
            * todo: something wrong with gamma pointer...
            auto g = qgraphicsitem_cast<GammaLine*>(kid);
            if (g) {
                //auto a = g->objectName();
                qDebug() << "g->objectName()";
                gammaline = g;
            }
            */

            auto h = qgraphicsitem_cast<QGraphicsPathItem*>(kid);;
            if (h && h->path().elementCount() == 0) {
                histogram = h;
            }
        }
    }
    QDoubleSpinBox* spinmin;
    QDoubleSpinBox* spinmax;
    QDoubleSpinBox* spingamma;
    ContrastWidgetImpl::ContrastChart* chart;
    VertLine* minline;
    VertLine* maxline;
    QGraphicsPathItem* histogram;
};
    

void tContrastWidget::init() {
    widget = new ContrastWidget;
    ptrs = new implptrs(widget);
}
void tContrastWidget::cleanup() {
    delete widget;
    widget = nullptr;

    delete ptrs;
    ptrs = nullptr;
}
void tContrastWidget::tsetgetcontrast() {
    ROIVert::contrast exp({ .1,.2,.3 });
    widget->setContrast(exp);
    ROIVert::contrast act{ widget->getContrast() };
    QCOMPARE(act, exp);
    QCOMPARE(ptrs->spinmin->text(), "0.10");
    QCOMPARE(ptrs->spinmax->text(), "0.20");
    QCOMPARE(ptrs->spingamma->text(), "0.30");

    QVERIFY(almostequal(ptrs->minline->getX(), .1));
    QVERIFY(almostequal(ptrs->maxline->getX(), .2));
}
void tContrastWidget::tsetspinners() {
    {
        double exp = .1;
        ptrs->spinmin->setValue(exp);
        ROIVert::contrast c{ widget->getContrast() };
        QVERIFY( almostequal(std::get<0>(c), exp) );
        QCOMPARE(ptrs->chart->getValues(), c);
    }
    {
        double exp = .2;
        ptrs->spinmax->setValue(exp);
        ROIVert::contrast c{ widget->getContrast() };
        QVERIFY( almostequal(std::get<1>(c), exp) );
        QCOMPARE(ptrs->chart->getValues(), c);
    }
    {
        double exp = .3;
        ptrs->spingamma->setValue(exp);
        ROIVert::contrast c{ widget->getContrast() };
        QVERIFY( almostequal(std::get<2>(c), exp) );
        QCOMPARE(ptrs->chart->getValues(), c);
    }
}


void tContrastWidget::thistogram() {
    widget->setHistogram({ .1f, .1f, .4f, .3f, .2f });    
    QVERIFY(ptrs->histogram != nullptr);
    QCOMPARE(ptrs->histogram->path().elementCount(), 7);    
    QVERIFY(almostequal(ptrs->histogram->path().elementAt(0).x, 0));
    QVERIFY(almostequal(ptrs->histogram->path().elementAt(0).y, 1));
    QVERIFY(almostequal(ptrs->histogram->path().elementAt(1).x, 0));
    QVERIFY(almostequal(ptrs->histogram->path().elementAt(1).y, .75));
    QVERIFY(almostequal(ptrs->histogram->path().elementAt(5).x,.8));
    QVERIFY(almostequal(ptrs->histogram->path().elementAt(5).y, .5));
    QVERIFY(almostequal(ptrs->histogram->path().elementAt(6).x, 1));
    QVERIFY(almostequal(ptrs->histogram->path().elementAt(6).y, 1));
    widget->setHistogramColor(QColor(100, 150, 200));
    QCOMPARE(ptrs->histogram->pen().color(), QColor(100, 150, 200));
}

void tContrastWidget::tlinecolor() {
    auto clr = QColor(50, 60, 70);
    widget->setLineColor(clr);
    
    QCOMPARE(ptrs->minline->pen().color(), clr);
    QCOMPARE(ptrs->maxline->pen().color(), clr);
}

void tContrastWidget::tchart2spin() {
    ptrs->minline->setX(.2);
    ptrs->maxline->setX(.7);
    QCOMPARE(ptrs->spinmin->text(), "0.20");
    QCOMPARE(ptrs->spinmax->text(), "0.70");
}
void tContrastWidget::tsignal() {
    bool fired = false;
    connect(widget, &ContrastWidget::contrastChanged, [&]() { fired = true; });

    // Set programmatically:
    widget->setContrast({ 1,2,3 });
    QVERIFY(fired);

    // Set spinner
    fired = false;
    ptrs->spinmax->setValue(.9);
    QVERIFY(fired);

    // Set chart
    fired = false;
    ptrs->minline->setX(.2);
    QVERIFY(fired);
}