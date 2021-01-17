/**
 * \class  ProjectionPickWidget.h
 *
 * \brief  Simple buttongroup widget for selecting projection (none/mean/min/max)
 *
 * \author neuroph
*/
#pragma once
#include <QWidget>

class QButtonGroup;

class ProjectionPickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectionPickWidget(QWidget* parent = nullptr);
    int getProjection();
    void setProjection(int projid);

signals:
    void projectionChanged();

private:
    QButtonGroup* projection;
};

