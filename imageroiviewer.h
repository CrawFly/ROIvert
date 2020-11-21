#pragma once

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "roi.h"

namespace ROIVert
{
	enum MODE
	{
		NONE,
		ADDROI,
		SELROI,
		ZOOM
	};
	enum ROISHAPE
	{
		RECTANGLE,
		ELLIPSE,
		POLYGON
	};
} // namespace ROIVert

struct MouseStatus
{
	ROIVert::MODE mode = ROIVert::NONE;
	size_t ActiveROI = 0;
	size_t ActiveVert = 0;
};

class ImageROIViewer : public QGraphicsView
{
	Q_OBJECT

public:
	explicit ImageROIViewer(QWidget *parent = nullptr);
	virtual ~ImageROIViewer();
	void setImage(const QImage image);
	QImage getImage();
	QSize getImageSize();
	void pushROI();
	void setMouseMode(ROIVert::MODE);
	void setROIShape(ROIVert::ROISHAPE);

	/*

	// Image stuff:
	// impl a setImage that takes a pointer to a mat?

	// ROI stuff
	void selectROI(const size_t index);											// 1-based index, with 0 referring to unselected

	// set/get all vertices
	void setROIVertices(const size_t index, const QVector<QPoint>& Vertices);	// Set the vertices for an roi
	QVector<QPoint> getROIVertices(const size_t index);							// Get the vertices
	
	// set/get a specific vertex
	void setROIVertex(const size_t roiIndex, const size_t vertexIndex, const QPoint& vertex);
	QPoint getROIVertex(const size_t roiIndex, const size_t vertexIndex);

	// set/get last vertex
	void setROILastVertex(const size_t roiIndex, const QPoint& vertex);
	QPoint getROILastVertex(const size_t roiIndex, const size_t vertexIndex);

	// Mask Stuff
	// void getMap(); // Map has a map of which pixels belong to which ROIs, notably with conflict resolved as the last roi added
	// QRect getBoundingBox(const size_t index);
	// cv::Mat getMask(const size_t index);			  // unbounded
	// cv::Mat getMask(const size_t index, QRect bb); // bounded
	// traces...

	*/
signals:
	void roiSelected(const size_t roiind);
	void roiEdited(const size_t roiind, const QVector<QPoint> &Vertices); // this fires on commit?
	void imgLoaded();
	void imgSizeChanged(const QSize newSize);

protected:
	void resizeEvent(QResizeEvent *event);
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	QImage img;
	QGraphicsScene *scene;
	QGraphicsPixmapItem *pixitem;

	bool hasImage = false;

	std::vector<roi *> rois;
	MouseStatus mousestatus;
	ROIVert::ROISHAPE roishape = ROIVert::RECTANGLE;
};