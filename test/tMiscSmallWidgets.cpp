#include <QtTest/QtTest>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>
#include <QCompleter>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>
#include <QLabel>

#include "tMiscSmallWidgets.h"
#include "widgets/SmoothingPickWidget.h"
#include "widgets/RGBWidget.h"
#include "widgets/ProjectionPickWidget.h"
#include "dockwidgets/FileIOWidget.h"
#include "ImageDataWindow.h"
#include "dockwidgets/ImageSettingsWidget.h"
#include "DisplaySettings.h"
#include "ImageDataTableModel.h"

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

void tMiscSmallWidgets::tFileIOWidget() {
    FileIOWidget w;
    w.testoverride = true;
    {
        auto cmd = w.findChild<QPushButton*>("cmdExpTraces");
        QString actfn;
        connect(&w, &FileIOWidget::exportTraces, [&](QString fn, bool, bool) {actfn = fn; });
        cmd->click();
        QCOMPARE(actfn, QDir::currentPath() + "/foo.csv");
    }
    {
        auto cmd = w.findChild<QPushButton*>("cmdExpROIs");
        QString actfn;
        connect(&w, &FileIOWidget::exportROIs, [&](QString fn) {actfn = fn; });
        cmd->click();
        QCOMPARE(actfn, QDir::currentPath() + "/foo.json");
    }
    {
        auto cmd = w.findChild<QPushButton*>("cmdImpROIs");
        QString actfn;
        connect(&w, &FileIOWidget::importROIs, [&](QString fn) {actfn = fn; });
        cmd->click();
        QCOMPARE(actfn, QDir::currentPath() + "/foo.json");
    }
    {
        auto cmd = w.findChild<QPushButton*>("cmdExpCharts");
        QString actfn;
        bool actisridge = false;
        connect(&w, &FileIOWidget::exportCharts, [&](QString fn, int, int, int, bool isridge) {actfn = fn; actisridge = isridge; });
        cmd->click();
        QCOMPARE(actfn, QDir::currentPath() + "/foo.jpeg");
        QVERIFY(!actisridge);
    }

    {
        auto cmd = w.findChild<QPushButton*>("cmdExpRidge");
        QString actfn;
        bool actisridge = false;
        connect(&w, &FileIOWidget::exportCharts, [&](QString fn, int, int, int, bool isridge) {actfn = fn; actisridge = isridge; });
        cmd->click();
        QCOMPARE(actfn, QDir::currentPath() + "/foo.jpeg");
        QVERIFY(actisridge);
    }
}

void tMiscSmallWidgets::tImageDataWindow() {
    ImageDataWindow w;
        
    auto cmdLoad = w.findChild<QPushButton*>("cmdLoad");
    auto folderview = w.findChild<QTreeView*>("folderview");
    auto fileview = w.findChild<QTableView*>("fileview");
    auto fsmodel = w.findChild<QFileSystemModel*>("fsmodel");
    auto immodel = w.findChild<ImageDataTableModel*>("immodel");
    auto lblDatasetInfo = w.findChild<QLabel*>("lblDatasetInfo");        
    auto spinFrameRate = w.findChild<QDoubleSpinBox*>("spinFrameRate");        
    auto spinDownTime = w.findChild<QSpinBox*>("spinDownTime");
    auto spinDownSpace = w.findChild<QSpinBox*>("spinDownSpace");


    
    QVERIFY(cmdLoad);
    QVERIFY(folderview);
    QVERIFY(fileview);
    QVERIFY(fsmodel);
    QVERIFY(immodel);
    QVERIFY(lblDatasetInfo);
    QVERIFY(spinFrameRate);
    QVERIFY(spinDownTime);
    QVERIFY(spinDownSpace);
    
    auto folderind = fsmodel->index(TEST_RESOURCE_DIR);
    folderview->setCurrentIndex(folderind);
    fileview->selectionModel()->clearSelection();
    auto fileind = immodel->getIndexFromName("roivert_testdata_onestack.tiff");
    fileview->selectionModel()->select(fileind, QItemSelectionModel::SelectionFlag::Select);
    
    spinFrameRate->setValue(1.);
    QString lbl = lblDatasetInfo->text();
    auto lbllist = lbl.split("\n");
    QCOMPARE(lbllist.size(), 3);
    QCOMPARE(lbllist[0], "Files Selected: 1");
    QCOMPARE(lbllist[1], "Frames: 8");
    QCOMPARE(lbllist[2], "Duration: 0:08");
    
    spinFrameRate->setValue(2.);
    lbl = lblDatasetInfo->text();
    lbllist = lbl.split("\n");
    QCOMPARE(lbllist.size(), 3);
    QCOMPARE(lbllist[0], "Files Selected: 1");
    QCOMPARE(lbllist[1], "Frames: 8");
    QCOMPARE(lbllist[2], "Duration: 0:04");


    // push button and check signal
    spinDownTime->setValue(2);
    spinDownSpace->setValue(3);
    bool waspressed{ false };
    std::vector<std::pair<QString, size_t>> fnfl;
    double fr;
    int dst, dss;

    connect(&w, &ImageDataWindow::fileLoadRequested, [&](std::vector<std::pair<QString, size_t>> filenameframelist, const double frameRate, const int dsTime, const int dsSpace) {
        waspressed = true;
        fnfl = filenameframelist;
        fr = frameRate;
        dst = dsTime;
        dss = dsSpace;
    });
    cmdLoad->pressed();

    QVERIFY(waspressed);
    QCOMPARE(fr, 2.);
    QCOMPARE(dst, 2);
    QCOMPARE(dss, 3);

    QString fp(TEST_RESOURCE_DIR);
    auto fn = fp + "/roivert_testdata_onestack.tiff";
    std::vector<std::pair<QString, size_t>> exp = { {fn, 8} };
    QCOMPARE(fnfl, exp);
}
void tMiscSmallWidgets::tImageSettingsWidget() {
    DisplaySettings sets;
    sets.setProjectionMode(3);

    ImageSettingsWidget w(nullptr, &sets);

    int cntr = 0;
    connect(&w, &ImageSettingsWidget::imgSettingsChanged, [&] {cntr++; });

    ROIVert::contrast c;
    w.setContrast(c);
    QCOMPARE(cntr, 1);

    auto butNone = w.findChild<QPushButton*>("butNone");
    auto butMean = w.findChild<QPushButton*>("butMean");
    butMean->click();
    QVERIFY(w.isProjectionActive());
    QCOMPARE(cntr, 2);
    butNone->click();
    QVERIFY(!w.isProjectionActive());
    QCOMPARE(cntr, 3);
}