#pragma once
#include <QGraphicsView>
#include <QGraphicsItem>
#include <qdebug.h>

// try to refactor this to use legit inheritence:
//		That means that thisroi has to live on the base
//			- which means some dynamic casting
//			- I should be able to impl getVertices() on the base
//			- Some of the constructor seems like it can live on the base...not sure about order


class roi {
public:
	virtual void setVertices(const QVector<QPoint>&) = 0;
	QVector<QPoint> getVertices() { return vertices; };
	virtual QRect getBB() = 0;
	
	// We'll adjust this later to pick a good pen...
	QPen getPen() {
		QPen pen(Qt::blue, 3);
		pen.setCosmetic(true);
		return pen;
	}
protected:
	QVector<QPoint> vertices;
};



class roi_rect : public roi { //regular roi
public:
	roi_rect(QGraphicsScene* scene);
	~roi_rect();
	virtual void setVertices(const QVector<QPoint>&);
	virtual QRect getBB();

private:
	QGraphicsRectItem* thisroi;
};
class roi_ellipse : public roi {
public:
	roi_ellipse(QGraphicsScene* scene);
	~roi_ellipse();
	virtual void setVertices(const QVector<QPoint>&);
	virtual QRect getBB();

private:
	QGraphicsEllipseItem* thisroi;
};
class roi_polygon : public roi {
public:
	roi_polygon(QGraphicsScene* scene);
	~roi_polygon();
	virtual void setVertices(const QVector<QPoint>&);
	virtual QRect getBB();

private:
	QGraphicsPolygonItem* thisroi;
};