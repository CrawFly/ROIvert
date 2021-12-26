#pragma once
#include <QWidget>

class ChartControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartControlWidget(QWidget* parent = nullptr);
    ~ChartControlWidget();
    
signals:
    void heightChanged(int newheight);

public slots:
    void changeMinimumHeight(int minheight);
    void changeHeight(int newheight);

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};
