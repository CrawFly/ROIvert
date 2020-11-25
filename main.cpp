#include "roivert.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    //int id = qRegisterMetaType<std::vector<double>>();

    QApplication a(argc, argv);
    Roivert w;
    w.show();
    return a.exec();
}
