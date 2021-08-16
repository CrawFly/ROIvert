#pragma once
#include <QGraphicsView>
#include <QGraphicsItem>
#include <qdebug.h>
#include "opencv2/opencv.hpp"

// try to refactor this to use legit inheritence:
//		That means that thisroi has to live on the base
//			- which means some dynamic casting
//			- I should be able to impl getVertices() on the base
//			- Some of the constructor seems like it can live on the base...not sure about order

class roi
{
public:
	virtual ~roi() {};
	virtual void setVertices(const QVector<QPoint> &) = 0;
	QVector<QPoint> getVertices() { return vertices; };
	virtual QRect getBB() = 0;
	virtual cv::Mat getMask() = 0;
	virtual void setColor(QColor clr) = 0;

	QPen getPen()
	{
		QPen pen(QColor(0, 94, 64), 4);
		pen.setCosmetic(true);
		return pen;
	}
	virtual void setScene(QGraphicsScene* scene) = 0;


protected:
	QVector<QPoint> vertices;
};

class roi_rect : public roi
{ //regular roi
public:
	roi_rect(QGraphicsScene *scene = nullptr);
	~roi_rect();
	void setVertices(const QVector<QPoint> &) override;
	QRect getBB() override;
	void setColor(QColor clr) override { QPen pen = thisroi->pen(); pen.setColor(clr); thisroi->setPen(pen); };
	cv::Mat getMask() override;
	void setScene(QGraphicsScene* scene) override;

private:
	QGraphicsRectItem *thisroi;
};
class roi_ellipse : public roi
{
public:
	roi_ellipse(QGraphicsScene *scene = nullptr);
	~roi_ellipse();
	void setVertices(const QVector<QPoint> &) override;
	QRect getBB() override;
	cv::Mat getMask() override;
	void setColor(QColor clr) override { QPen pen = thisroi->pen(); pen.setColor(clr); thisroi->setPen(pen); }
	void setScene(QGraphicsScene* scene) override;

private:
	QGraphicsEllipseItem *thisroi;
};
class roi_polygon : public roi
{
public:
	roi_polygon(QGraphicsScene *scene = nullptr);
	~roi_polygon();
	void setVertices(const QVector<QPoint> &) override;
	QRect getBB() override;
	cv::Mat getMask() override;
	void setColor(QColor clr) override { QPen pen = thisroi->pen(); pen.setColor(clr); thisroi->setPen(pen); }
	void setScene(QGraphicsScene* scene) override;

private:
	QGraphicsPolygonItem *thisroi;
};