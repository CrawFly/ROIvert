#include "ProjectionPickWidget.h"
#include <QGridLayout>
#include <QPushButton>


enum class proj {
    NONE = 0,
    MIN = 1,
    MAX = 2,
    MEAN = 3
};

ProjectionPickWidget::ProjectionPickWidget(QWidget* parent) noexcept : QWidget(parent) {
    QGridLayout* Lay = new QGridLayout;

    QPushButton* butNone = new QPushButton("None");
    butNone->setCheckable(true);
    butNone->setChecked(true);
    QPushButton* butMean = new QPushButton("Mean");
    butMean->setCheckable(true);
    QPushButton* butMin = new QPushButton("Min");
    butMin->setCheckable(true);
    QPushButton* butMax = new QPushButton("Max");
    butMax->setCheckable(true);

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
