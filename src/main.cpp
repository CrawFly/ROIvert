#include "roivert.h"

#include <QDebug>
#include <QtWidgets/QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QDesktopWidget>
#include <QPainter>
#include <QGUIApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    QRect screenGeometry(0, 0, 1920, 1080);
    
    auto screens = QGuiApplication::screens();
    if (!screens.empty()) {
        screenGeometry=screens[0]->geometry();
    }

    //QRect screenGeometry = desktopWidget->screenGeometry();
    const int screenWidth = screenGeometry.width();
    const int screenHeight = screenGeometry.height();

    QPixmap pixmap(":/splash.png");
    float sf = 4000./screenWidth;

    pixmap = pixmap.scaled(pixmap.width()/sf, pixmap.height()/sf);

    QPainter painter( &pixmap );
    painter.setPen(QPen(Qt::white));
    painter.drawText( QPoint(pixmap.width()*.78, pixmap.height()*.79), "V" ROIVERTVERSION);

    QSplashScreen splash(pixmap);
    splash.show();

    Roivert w;
    QTimer t;

    const int delay = 1500;
    t.singleShot(delay, [&]
                 {
                     w.show();
                     w.setInitialSettings();
                     splash.finish(&w);
                 });

    return a.exec();
}