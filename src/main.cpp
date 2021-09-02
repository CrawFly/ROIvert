#include "roivert.h"

#include <QtWidgets/QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDesktopWidget* desktopWidget = qApp->desktop();
    QRect screenGeometry = desktopWidget->screenGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    QPixmap pixmap(":/splash.png");
    auto sf = 4000/screenWidth;

    pixmap = pixmap.scaled(pixmap.width()/sf, pixmap.height()/sf);


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