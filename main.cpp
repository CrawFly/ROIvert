#include "roivert.h"
#include <QtWidgets/QApplication>
#include <QSplashScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    //int id = qRegisterMetaType<std::vector<double>>();

    QApplication a(argc, argv);
    QPixmap pixmap(":/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    
    // todo: roivert construction currently shows the child windows, which means they pop up before the splash is gone
    //      break these out of constructor?
    Roivert w;
    
    QTimer t;
    t.singleShot(1500, [&] { w.show(); splash.finish(&w); });
    return a.exec();
}