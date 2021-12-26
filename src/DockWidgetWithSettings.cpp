#include "DockWidgetWithSettings.h"
#include <QToolBar>

DockWidgetWithSettings::DockWidgetWithSettings(QWidget* parent) : QDockWidget(parent)
{
    auto contents = new QWidget;
    this->setWidget(contents);
    contents->setLayout(&toplay);

    auto tb = new QToolBar;
    reset = new QAction(QIcon(":/icons/dockreset.png"), "");
    save = new QAction(QIcon(":/icons/docksave.png"), "");
    reset->setToolTip(tr("Reset settings for this window."));
    save->setToolTip(tr("Toggle whether settings for this window carried over to the next session."));
    save->setCheckable(true);
    save->setChecked(true);

    auto blank = new QWidget;
    blank->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tb->addWidget(blank);
    tb->addAction(reset);
    tb->addAction(save);
    tb->setIconSize(QSize(24, 36));
    tb->setFixedHeight(36);

    toplay.addWidget(tb);
    toplay.setContentsMargins(0, 0, 0, 0);
    toplay.setSpacing(0);

    connect(reset, &QAction::triggered, this, &DockWidgetWithSettings::resetSettings);
    connect(this, &QDockWidget::topLevelChanged, this, [=] { this->adjustSize(); });
}
bool DockWidgetWithSettings::getSettingsStorage() const {
    return save->isChecked();
}
void DockWidgetWithSettings::setSettingsStorage(bool yesno) {
    save->setChecked(yesno);
}