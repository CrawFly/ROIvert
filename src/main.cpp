#include "roivert.h"

#include <QDebug>
#include <QtWidgets/QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QDesktopWidget>
#include <QPainter>
#include <QGUIApplication>
#include <QScreen>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    Roivert w;

#ifdef NDEBUG
    QRect screenGeometry(0, 0, 1920, 1080);
    auto screens = QGuiApplication::screens();
    if (!screens.empty()) {
        screenGeometry = screens[0]->geometry();
    }

    //QRect screenGeometry = desktopWidget->screenGeometry();
    const int screenWidth = screenGeometry.width();
    const int screenHeight = screenGeometry.height();

    QPixmap pixmap(":/splash.png");
    float sf = 4000. / screenWidth;

    pixmap = pixmap.scaled(pixmap.width() / sf, pixmap.height() / sf);

    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::white));
    painter.drawText(QRect(0, pixmap.height() * .75, pixmap.width(), pixmap.height() * .1), Qt::AlignCenter & Qt::AlignHCenter, "V" ROIVERTVERSION);
    QSplashScreen splash(pixmap);
    splash.show();
    QTimer t;
    const int delay = 1500;
    t.singleShot(delay, [&]
    {
        w.show();
        w.setInitialSettings();
        splash.finish(&w);
    });
#else
    w.show();
#endif

    return a.exec();
}