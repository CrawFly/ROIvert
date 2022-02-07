#pragma once
#include <QObject>

class FileIO;

class tFileIO : public QObject
{
    Q_OBJECT
public:
    tFileIO();

private slots:
    void init();
    void cleanup();
    void texporttraces();
    void texportrois();
    void timportrois();
    void texportcharts_data();
    void texportcharts();

private:
    struct objptrs;
    objptrs* ptrs;
    FileIO* fileio;
};
// void exportROIs(QString filename) const;
// void exportCharts(QString filename, int width, int height, int quality, bool ridge);
