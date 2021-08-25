#pragma once
#include <QDockWidget>

class FileIOWidget : public QDockWidget
{    
Q_OBJECT

public:
    FileIOWidget(QWidget* parent = nullptr);
    void setContentsEnabled(bool);

signals:
    void exportTraces(QString filename, bool doHeader, bool doTimeCol);
    void exportROIs(QString filename);
    void importROIs(QString filename);
    void exportCharts(QString filename, int width, int height, int quality, bool ridge);

private:
    QWidget* contents;
};

