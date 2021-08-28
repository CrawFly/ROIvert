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
    
    Roivert w;
    QTimer t;

    int delay = 1500;
    t.singleShot(delay, [&] { 
        w.show();  
        w.restoreSettings();    
        splash.finish(&w); 
    });
    return a.exec();
}