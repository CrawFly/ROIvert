#include <QtTest/QtTest>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QDial>
#include <QLabel>
#include "tVideoControllerWidget.h"
#include "widgets/VideoControllerWidget.h"


void tVideoControllerWidget::init() {
    widget = new VideoControllerWidget;

    
    sliScrub = widget->findChild<QSlider*>("sliScrub");
    cmdBack = widget->findChild<QPushButton*>("cmdBack");
    cmdPlay = widget->findChild<QPushButton*>("cmdPlay");
    cmdForw = widget->findChild<QPushButton*>("cmdForw");
    cmdLoop = widget->findChild<QPushButton*>("cmdLoop");
    cmdDff = widget->findChild<QPushButton*>("cmdDff");
    txtSpeed = widget->findChild<QLineEdit*>("txtSpeed");
    dialSpeed = widget->findChild<QDial*>("dialSpeed");
    lblTime = widget->findChild<QLabel*>("lblTime");

}
void tVideoControllerWidget::cleanup() {
    delete widget ;
    widget = nullptr;
}

void tVideoControllerWidget::ttiming_data() {
    QTest::addColumn<float>("fr");
    QTest::newRow("slow") << 10.f;
    QTest::newRow("medium") << 30.f;
    QTest::newRow("fast") << 100.f;
    QTest::newRow("fractional") << 33.33f;
}

void tVideoControllerWidget::tspeeddial_data() {
    QTest::addColumn<int>("val");
    QTest::addColumn<QString>("txt");
    QTest::newRow("slow1") << -50 << "0.1";
    QTest::newRow("slow2") << -24 << "0.33";
    QTest::newRow("act") << 0 << "1";
    QTest::newRow("fast1") << 30 << "4";
    QTest::newRow("fast2") << 60 << "16";
}
void tVideoControllerWidget::ttimelabel_data() {
    QTest::addColumn<int>("nframes");
    QTest::addColumn<float>("framerate");
    QTest::addColumn<int>("currframe");
    QTest::addColumn<QString>("expstr");
    QTest::newRow("A") << 100 << 30.f << 10 << "00:300 (10/100)";
    QTest::newRow("B") << 100 << 30.f << 20 << "00:633 (20/100)";
    QTest::newRow("C") << 100 << 30.f << 30 << "00:966 (30/100)";
    QTest::newRow("D") << 1000 << 10.f << 1 << "00:000 (1/1000)";
    QTest::newRow("E") << 1000 << 10.f << 500 << "49:900 (500/1000)";
    QTest::newRow("F") << 1000 << 10.f << 1000 << "01:39:900 (1000/1000)";
}

void tVideoControllerWidget::ttiming(){
    QFETCH(float, fr);
    size_t n = 100;
    auto expected_interval = 1000 / fr;
    int timeout = std::ceil(n / fr)*1000 + 1000;

    widget->setNFrames(n);
    widget->setFrameRate(fr);
    QElapsedTimer signaltimer;

    signaltimer.start();
    
    std::vector<int> elapsed_time;
    elapsed_time.reserve(n);
    elapsed_time.push_back(0);
    
    std::vector<size_t> elapsed_frames;
    elapsed_frames.reserve(n);
    elapsed_frames.push_back(0);
    
    connect(widget, &VideoControllerWidget::frameChanged, [&](const size_t& frame) { 
        elapsed_time.push_back(signaltimer.restart()); 
        elapsed_frames.push_back(frame);
    });
    widget->start();
    QTRY_VERIFY_WITH_TIMEOUT(widget->getCurrFrame() == n, timeout);

    // Average elapsed time should be about expected_interval (note that this uses n)
    auto avginterval = std::accumulate(elapsed_time.begin(), elapsed_time.end(), 0) / n;
    QVERIFY(std::abs(avginterval - expected_interval) < 5); // Average

    // VideoControllerWidget should know to skip frames if there's not enough time to show them:
    float maxinterval = 0;
    for (size_t i = 1; i < elapsed_frames.size(); ++i) {
        float nfr = elapsed_frames[i] - elapsed_frames[i-1];
        float dur = elapsed_time[i];
        float thisrate = dur / nfr;
        maxinterval = std::max(dur / nfr, maxinterval);
    }
    QVERIFY(maxinterval < expected_interval * 2);
}
void tVideoControllerWidget::tdff() {
    size_t dffToggledFireCount = 0;
    bool checkedstate{ false };
    size_t frameChangedFireCount = 0;
    size_t framestate = 0;


    connect(widget, &VideoControllerWidget::dffToggled, [&](bool chk) {checkedstate = chk; dffToggledFireCount++; } );
    connect(widget, &VideoControllerWidget::frameChanged, [&](size_t currframe) {framestate = currframe; frameChangedFireCount++; } );

    widget->dffToggle(true);
    QVERIFY(widget->isDff());
    QVERIFY(checkedstate);
    QCOMPARE(dffToggledFireCount, 1);
    QCOMPARE(frameChangedFireCount, 1);
    QCOMPARE(framestate, 0);
    QVERIFY(cmdDff->isChecked());

    cmdDff->click();
    QVERIFY(!checkedstate);
    QCOMPARE(dffToggledFireCount, 2);
    QCOMPARE(frameChangedFireCount, 2);
}

void tVideoControllerWidget::tenabled() {
    widget->setNFrames(10);
    QVERIFY(widget->isEnabled());
    widget->setNFrames(0);
    QVERIFY(!widget->isEnabled());
}

void tVideoControllerWidget::tloop() {
    // set the looper on. set frame make sure we eventually hit frame before
    int timeout = 1500;

    widget->setNFrames(10);
    QCOMPARE(sliScrub->maximum(), 10);  // required for next steps
    widget->setFrameRate(30);           // this should be slow enough to ensure all frames get hit
    sliScrub->setValue(5);
    cmdLoop->setChecked(true);

    // now if we start playing should eventually hit 4...
    widget->start();
    QTRY_VERIFY_WITH_TIMEOUT(widget->getCurrFrame() == 4, timeout);
    widget->stop();
}

void tVideoControllerWidget::tsetnframes() {
    widget->setNFrames(4);
    QCOMPARE(sliScrub->maximum(), 4);
    sliScrub->setValue(3);
    QCOMPARE(widget->getCurrFrame(), 3);
    
    widget->setNFrames(3);
    QCOMPARE(widget->getCurrFrame(), 1); // Expect currframe reset.
    QCOMPARE(sliScrub->maximum(), 3);
}
void tVideoControllerWidget::tspeeddial() {
    QFETCH(int, val);
    QFETCH(QString, txt);
    dialSpeed->setValue(val);
    QCOMPARE(txt, txtSpeed->text());
    
    dialSpeed->setValue(-100);
    txtSpeed->setText(txt);
    txtSpeed->editingFinished();
    QCOMPARE(val, dialSpeed->value());
}
void tVideoControllerWidget::ttimelabel() {
    QFETCH(int, nframes);
    QFETCH(float, framerate);
    QFETCH(int, currframe);
    QFETCH(QString, expstr);
    
    widget->setNFrames(nframes);
    widget->setFrameRate(framerate);
    sliScrub->setValue(currframe);
   
    QCOMPARE(lblTime->text(), expstr);
}

void tVideoControllerWidget::tforwback() {
    widget->setNFrames(5);
    sliScrub->setValue(5);

    size_t frameChangedFireCount = 0;
    connect(widget, &VideoControllerWidget::frameChanged, [&](size_t currframe) { frameChangedFireCount++; } );

    QCOMPARE(widget->getCurrFrame(), 5);
    cmdBack->click();
    QCOMPARE(widget->getCurrFrame(), 4);
    QCOMPARE(frameChangedFireCount, 1);

    cmdForw->click();
    QCOMPARE(widget->getCurrFrame(), 5);
    QCOMPARE(frameChangedFireCount, 2);
    
    cmdForw->click();
    QCOMPARE(frameChangedFireCount, 2); // at the end, no fire    
}

void tVideoControllerWidget::tframeoverflow() {
    // loop is off, but next step would be past max frame, should stop at max frame
    widget->setNFrames(5);
    widget->setFrameRate(100000);
    sliScrub->setValue(4);
    widget->start();
    QTRY_VERIFY_WITH_TIMEOUT(widget->getCurrFrame() == 5, 100);
}

void tVideoControllerWidget::tplaybutton() {
    widget->setNFrames(10);
    widget->setFrameRate(20);

    cmdPlay->click();
    QTRY_VERIFY_WITH_TIMEOUT(widget->getCurrFrame(), 1000);
    cmdPlay->click();

    auto stopframe = widget->getCurrFrame();

    QTime dieTime= QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
        
    QCOMPARE(widget->getCurrFrame(), stopframe);
}