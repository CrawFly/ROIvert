#include "tSettings.h"
#include <QtTest/QtTest>
#include "opencv2/opencv.hpp"

#include "roivert.h"
#include "ROIVertSettings.h"
#include "dockwidgets/ImageDataWidget.h"
#include "dockwidgets/ImageSettingsWidget.h"
#include "dockwidgets/StyleWidget.h"
#include "dockwidgets/FileIOWidget.h"

#include <QElapsedTimer>
#include <QSpinBox>
#include <QDoubleSpinBox>

void tSettings::init() {
    // NOTE: tSettings is 'destructive' but settins will use a application name ROIVertTest,
    // so this shouldn't impact regular ROIVert installed on the same machine

    // Always clear any previously set settings at startup
    r = new Roivert;
    r->setInitialSettings(false);

    auto idw = r->findChild<ImageDataWidget*>();
    auto isw = r->findChild<ImageSettingsWidget*>();
    auto sw = r->findChild<StyleWidget*>();
    auto fw = r->findChild<FileIOWidget*>();
    rsettings = new ROIVertSettings(r, idw, isw, sw, fw);

    // Reset saved settings
    QSettings().clear();
    rsettings->saveSettings();

    // Clear settings
    QSettings().clear();
}

void tSettings::cleanup() {
    delete(rsettings);
    rsettings = nullptr;
    delete(r);
    r = nullptr;
}

void tSettings::checkWidgetValuesDefault() {
    auto spinChartQuality = r->findChild<QSpinBox*>("spinChartQuality");
    auto spinFrameRate = r->findChild<QDoubleSpinBox*>("spinFrameRate");
    auto isw = r->findChild<ImageSettingsWidget*>();
    auto linewidth = r->findChild<QSpinBox*>("linewidth");

    QCOMPARE(spinChartQuality->value(), 100);
    QCOMPARE(spinFrameRate->value(), 30);
    QCOMPARE(isw->getContrast(), ROIVert::contrast({ 0, 1, 1 }));
    QCOMPARE(linewidth->value(), 2);
}
void tSettings::checkWidgetValuesCustom() {
    auto spinChartQuality = r->findChild<QSpinBox*>("spinChartQuality");
    auto spinFrameRate = r->findChild<QDoubleSpinBox*>("spinFrameRate");
    auto isw = r->findChild<ImageSettingsWidget*>();
    auto linewidth = r->findChild<QSpinBox*>("linewidth");

    QCOMPARE(spinChartQuality->value(), 42);
    QCOMPARE(spinFrameRate->value(), 23);
    QCOMPARE(isw->getContrast(), ROIVert::contrast({ .1, .2, 2 }));
    QCOMPARE(linewidth->value(), 5);
}

void tSettings::applyCustomValuesToWidgets() {
    auto spinChartQuality = r->findChild<QSpinBox*>("spinChartQuality");
    auto spinFrameRate = r->findChild<QDoubleSpinBox*>("spinFrameRate");
    auto isw = r->findChild<ImageSettingsWidget*>();
    auto linewidth = r->findChild<QSpinBox*>("linewidth");

    spinChartQuality->setValue(42);
    spinFrameRate->setValue(23);
    ROIVert::contrast c({ .1, .2, 2 });
    isw->setContrast(c);
    linewidth->setValue(5);
}

void tSettings::tsavesettings() {
    QVERIFY(QSettings().allKeys().isEmpty());
    applyCustomValuesToWidgets();

    rsettings->saveSettings();

    QSettings s;
    auto groups = s.childGroups();
    QCOMPARE(groups.length(), 5);
    QVERIFY(groups.contains("FileIO"));
    QVERIFY(groups.contains("ImageData"));
    QVERIFY(groups.contains("ImageSettings"));
    QVERIFY(groups.contains("Style"));
    QVERIFY(groups.contains("window"));

    QCOMPARE(s.value("FileIO/chartquality").toInt(), 42);
    QCOMPARE(s.value("ImageData/framerate").toDouble(), 23);
    QVERIFY(std::abs(s.value("ImageSettings/rawCont0").toDouble() - .1) < .00001);
    QVERIFY(std::abs(s.value("ImageSettings/rawCont1").toDouble() - .2) < .00001);
    QVERIFY(std::abs(s.value("ImageSettings/rawCont2").toDouble() - 2.) < .00001);

    QVERIFY(s.contains("version"));
    QCOMPARE(s.value("version").toString(), ROIVERTVERSION);
}
void tSettings::tresetsettings() {
    checkWidgetValuesDefault();
    applyCustomValuesToWidgets();
    checkWidgetValuesCustom();
    rsettings->resetSettings();
    checkWidgetValuesDefault();
}
void tSettings::trestoresettings() {
    checkWidgetValuesDefault();
    applyCustomValuesToWidgets();
    checkWidgetValuesCustom();
    rsettings->saveSettings();
    rsettings->resetSettings();
    checkWidgetValuesDefault();
    rsettings->restoreSettings();
    checkWidgetValuesCustom();
}