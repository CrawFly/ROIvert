#include "DockWidgetWithSettings.h"
#include <QToolBar>
#include <QObject>

Settingsable::Settingsable()
{
    auto tb = new QToolBar;
    reset = new QAction(QIcon::fromTheme("set_reset"), "");
    save = new QAction(QIcon::fromTheme("set_save"), "");
    reset->setToolTip(QObject::tr("Reset settings for this window."));
    save->setToolTip(QObject::tr("Toggle whether settings for this window carried over to the next session."));
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
 
    QObject::connect(reset, &QAction::triggered, [=]() { this->resetSettings(); });
}

bool Settingsable::getSettingsStorage() const
{
    return save->isChecked();
}

void Settingsable::setSettingsStorage(bool yesno)
{
    save->setChecked(yesno);
}