#include "ProjectionPickWidget.h"
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>

#include "roivertcore.h"

enum class proj {
    NONE = 0,
    MIN = 1,
    MAX = 2,
    MEAN = 3
};

ProjectionPickWidget::ProjectionPickWidget(QWidget* parent) : QWidget(parent) {
    projection = new QButtonGroup;
    QGridLayout* Lay = new QGridLayout;

    QPushButton* butNone = new QPushButton("None");
    butNone->setCheckable(true);
    butNone->setChecked(true);
    butNone->setToolTip(tr("Show individual frames."));

    QPushButton* butMean = new QPushButton("Mean");
    butMean->setCheckable(true);
    butMean->setToolTip("Show pixel-wise average across frames (undefined for " + ROIVert::dffstring() + ".");
    
    QPushButton* butMin = new QPushButton("Min");
    butMin->setCheckable(true);
    butNone->setToolTip(tr("Show pixel-wise minimum across frames."));

    QPushButton* butMax = new QPushButton("Max");
    butMax->setCheckable(true);
    butNone->setToolTip(tr("Show pixel-wise maximum across frames."));

    projection->addButton(butNone, static_cast<int>(proj::NONE));
    projection->addButton(butMean, static_cast<int>(proj::MEAN));
    projection->addButton(butMin, static_cast<int>(proj::MIN));
    projection->addButton(butMax, static_cast<int>(proj::MAX));

    Lay->addWidget(butNone, 0, 0);
    Lay->addWidget(butMean, 0, 1);
    Lay->addWidget(butMin, 1, 0);
    Lay->addWidget(butMax, 1, 1);

    Lay->setSpacing(0);
    this->setLayout(Lay);

    connect(projection, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &ProjectionPickWidget::projectionChanged);
}
int ProjectionPickWidget::getProjection() {return projection->checkedId();}
void ProjectionPickWidget::setProjection(int projid) {projection->button(projid)->setChecked(true);}
