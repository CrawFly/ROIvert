#include "widgets/ProjectionPickWidget.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QPushButton>

#include "roivertcore.h"

enum class proj
{
    NONE = 0,
    MIN = 1,
    MAX = 2,
    MEAN = 3
};

struct ProjectionPickWidget::pimpl
{
    QButtonGroup projection;
    QGridLayout layout;
    QPushButton butNone{tr("None")};
    QPushButton butMean{"Mean"};
    QPushButton butMin{tr("Min")};
    QPushButton butMax{tr("Max")};

    void init()
    {
        butNone.setCheckable(true);
        butNone.setChecked(true);
        butNone.setToolTip(tr("Show individual frames."));

        butMean.setCheckable(true);
        butMean.setToolTip(tr("Show pixel-wise average across frames (undefined for ") + ROIVert::dffstring() + ".");

        butMin.setCheckable(true);
        butNone.setToolTip(tr("Show pixel-wise minimum across frames."));

        butMax.setCheckable(true);
        butNone.setToolTip(tr("Show pixel-wise maximum across frames."));

        projection.addButton(&butNone, static_cast<int>(proj::NONE));
        projection.addButton(&butMean, static_cast<int>(proj::MEAN));
        projection.addButton(&butMin, static_cast<int>(proj::MIN));
        projection.addButton(&butMax, static_cast<int>(proj::MAX));

    }

    void doLayout()
    {
        layout.setSpacing(0);
        layout.setContentsMargins(0, 0, 0, 0);
        layout.addWidget(&butNone, 0, 0);
        layout.addWidget(&butMean, 0, 1);
        layout.addWidget(&butMin, 1, 0);
        layout.addWidget(&butMax, 1, 1);
    }
};

ProjectionPickWidget::ProjectionPickWidget(QWidget *parent) : QWidget(parent)
{
    
    impl->init();
    impl->doLayout();
    setLayout(&impl->layout);
    connect(&impl->projection, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &ProjectionPickWidget::projectionChanged);
}
int ProjectionPickWidget::getProjection() const noexcept { return impl->projection.checkedId(); }
void ProjectionPickWidget::setProjection(int projid) { impl->projection.button(projid)->setChecked(true); }
