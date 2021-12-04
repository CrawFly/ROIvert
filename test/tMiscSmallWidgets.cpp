#include <QtTest/QtTest>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QButtonGroup>
#include <QPushButton>
#include "tMiscSmallWidgets.h"
#include "widgets/SmoothingPickWidget.h"
#include "widgets/RGBWidget.h"
#include "widgets/ProjectionPickWidget.h"

void tMiscSmallWidgets::tSmoothingPickWidget_data() {

    QTest::addColumn<int>("type");
    QTest::addColumn<int>("size");
    QTest::addColumn<float>("sig");
    QTest::addColumn<float>("sigi");
    QTest::addColumn<bool>("paramsvis");
    QTest::addColumn<bool>("sigvis");
    QTest::addColumn<bool>("sigivis");


    QTest::newRow("none")       << 0 << 1 << 2.f << 3.f << false << false << false;
    QTest::newRow("box")        << 1 << 2 << 3.f << 4.f << true  << false << false;
    QTest::newRow("median")     << 2 << 3 << 4.f << 5.f << true  << false << false;
    QTest::newRow("gaussian")   << 3 << 4 << 5.f << 6.f << true  << true  << false;
    QTest::newRow("bilateral")  << 4 << 5 << 6.f << 7.f << true  << true  << true;
}
void tMiscSmallWidgets::tSmoothingPickWidget()
{
    auto widget = std::make_unique<SmoothingPickWidget>();

    QFETCH(int, type);
    QFETCH(int, size);
    QFETCH(float, sig);
    QFETCH(float, sigi);
    QFETCH(bool, paramsvis);
    QFETCH(bool, sigvis);
    QFETCH(bool, sigivis);

    ROIVert::smoothing s = { type, size, sig, sigi };
    widget->setSmoothing(s);
    QCOMPARE(widget->getSmoothing(), s);
    
    auto cmb = widget->findChild<QComboBox*>("cmbBlur");
    QCOMPARE(cmb->currentIndex(), type);

    widget->updateSmothingParamWidgets();

    QCOMPARE(widget->findChild<QWidget*>("widgParams")->isVisibleTo(widget.get()), paramsvis);
    QCOMPARE(widget->findChild<QWidget*>("spinBlurSize")->isVisibleTo(widget.get()), paramsvis);
    QCOMPARE(widget->findChild<QWidget*>("lblSigma")->isVisibleTo(widget.get()), sigvis);
    QCOMPARE(widget->findChild<QWidget*>("spinBlurSigma")->isVisibleTo(widget.get()), sigvis);
    QCOMPARE(widget->findChild<QWidget*>("lblSigmaI")->isVisibleTo(widget.get()), sigivis);
    QCOMPARE(widget->findChild<QWidget*>("spinBlurSigmaI")->isVisibleTo(widget.get()), sigivis);
}
void tMiscSmallWidgets::tSmoothingPickWidgetSignal() 
{
    auto widget = std::make_unique<SmoothingPickWidget>();
    auto cmb = widget->findChild<QComboBox*>("cmbBlur");
    int firecount = 0;
    connect(widget.get(), &SmoothingPickWidget::smoothingChanged, [&] {firecount++; });
    
    widget->findChild<QComboBox*>("cmbBlur")->activated(2);
    QCOMPARE(firecount, 1);
    widget->findChild<QSpinBox*>("spinBlurSize")->valueChanged(2);
    QCOMPARE(firecount, 2);
    widget->findChild<QDoubleSpinBox*>("spinBlurSigma")->valueChanged(3.);
    QCOMPARE(firecount, 3);
    widget->findChild<QDoubleSpinBox*>("spinBlurSigmaI")->valueChanged(4.);
    QCOMPARE(firecount, 4);
}
void tMiscSmallWidgets::tRGBWidget()
{
    auto widget = std::make_unique<RGBWidget>();
    int firecount = 0;
    connect(widget.get(), &RGBWidget::colorChanged, [&] {firecount++; });
    widget->setColor(QColor(30, 60, 90));
    QCOMPARE(widget->getColor(), QColor(30, 60, 90));
    QCOMPARE(firecount, 1);
}
void tMiscSmallWidgets::tProjectionPickWidget()
{
    auto widget = std::make_unique<ProjectionPickWidget>();
    int firecount = 0;
    connect(widget.get(), &ProjectionPickWidget::projectionChanged, [&] {firecount++; });
    widget->setProjection(2);
    auto bg = widget->findChild<QPushButton*>()->group();
    bg->idClicked(2);
    
    QCOMPARE(widget->getProjection(), 2);
    QCOMPARE(firecount, 1);
}