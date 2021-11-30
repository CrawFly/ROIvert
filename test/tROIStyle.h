#pragma once
#include <QObject>
#include "QtTest/QtTest"
#include "ROI/ROIStyle.h"

class tROIStyle : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void tpen();
    void tbrush();
    void tselsize();
    void tcolorbyselected();
    void tpalettecolors();
    void tcopy();

private:
    ROIStyle* style;
};

