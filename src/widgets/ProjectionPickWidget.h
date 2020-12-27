/**
 * \class  ProjectionPickWidget.h
 *
 * \brief  Simple buttongroup widget for selecting projection (none/mean/min/max)
 *
 * \author neuroph
*/
#pragma once

#include <QWidget>
#include <QButtonGroup>

class ProjectionPickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectionPickWidget(QWidget* parent = nullptr) noexcept;
    int getProjection();
    void setProjection(int projid);

signals:
    void projectionChanged();

private:
    QButtonGroup* projection = new QButtonGroup;
};

