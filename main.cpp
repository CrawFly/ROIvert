#include "roivert.h"
#include <QtWidgets/QApplication>
#include <QSplashScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    //int id = qRegisterMetaType<std::vector<double>>();

    QApplication a(argc, argv);
    //todo: pack this into resources...if we're going forward with splash?
    QPixmap pixmap("C:\\Users\\dbulk\\OneDrive\\Documents\\qtprojects\\Roivert\\greenking_splash.png");
    QSplashScreen splash(pixmap);
    splash.show();

    Roivert w;
    
    QTimer t;
    t.singleShot(1500, [&] { w.show(); splash.finish(&w); });
    
    
    
    return a.exec();
}
