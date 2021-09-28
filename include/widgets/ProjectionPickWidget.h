/**
 * \class  ProjectionPickWidget.h
 *
 * \brief  Simple buttongroup widget for selecting projection (none/mean/min/max)
 *
 * \author neuroph
*/
#pragma once
#include <QWidget>

class ProjectionPickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectionPickWidget(QWidget* parent = nullptr);
    ~ProjectionPickWidget();
    
    int getProjection() const noexcept;
    void setProjection(int projid);

signals:
    void projectionChanged();

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl;
};

