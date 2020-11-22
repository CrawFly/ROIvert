#pragma once
#include <QGraphicsView>
#include <QGraphicsItem>
#include <qdebug.h>

// try to refactor this to use legit inheritence:
//		That means that thisroi has to live on the base
//			- which means some dynamic casting
//			- I should be able to impl getVertices() on the base
//			- Some of the constructor seems like it can live on the base...not sure about order

class roi
{
public:
	virtual void setVertices(const QVector<QPoint> &) = 0;
	QVector<QPoint> getVertices() { return vertices; };
	virtual QRect getBB() = 0;

	QPen getPen()
	{
		QPen pen(QColor(0, 94, 64), 4);
		pen.setCosmetic(true);
		return pen;
	}

	virtual void setColor(QColor clr) = 0;

protected:
	QVector<QPoint> vertices;
};

class roi_rect : public roi
{ //regular roi
public:
	roi_rect(QGraphicsScene *scene);
	~roi_rect();
	virtual void setVertices(const QVector<QPoint> &);
	virtual QRect getBB();
	void setColor(QColor clr) { QPen pen = thisroi->pen(); pen.setColor(clr); thisroi->setPen(pen); }

private:
	QGraphicsRectItem *thisroi;
};
class roi_ellipse : public roi
{
public:
	roi_ellipse(QGraphicsScene *scene);
	~roi_ellipse();
	virtual void setVertices(const QVector<QPoint> &);
	virtual QRect getBB();
	void setColor(QColor clr) { QPen pen = thisroi->pen(); pen.setColor(clr); thisroi->setPen(pen); }

private:
	QGraphicsEllipseItem *thisroi;
};
class roi_polygon : public roi
{
public:
	roi_polygon(QGraphicsScene *scene);
	~roi_polygon();
	virtual void setVertices(const QVector<QPoint> &);
	virtual QRect getBB();
	void setColor(QColor clr) { QPen pen = thisroi->pen(); pen.setColor(clr); thisroi->setPen(pen); }

private:
	QGraphicsPolygonItem *thisroi;
};