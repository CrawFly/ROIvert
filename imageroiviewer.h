#pragma once

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "roi.h"
#include "opencv2/opencv.hpp"
#include <QObject>

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

	void setImage(const QImage image);
	void setMouseMode(ROIVert::MODE);
	void setROIShape(ROIVert::ROISHAPE);

	void setSelectedROI(size_t ind);
	roi *getRoi(size_t ind) const;
	void deleteROI(size_t roiind);

	QVector<QPair<ROIVert::ROISHAPE, QVector<QPoint>>> getAllROIs() const;
	void importROIs(std::vector<std::unique_ptr<roi>> &rois_in);
	size_t getNROIs() const;

	void setSelectedColor(QColor clr);
	void setUnselectedColor(QColor clr);

signals:
	void roiSelectionChange(const size_t oldroiind, const size_t newroiind);
	void roiEdited(const size_t roiind);
	void toolfromkey(int key);
	void roiDeleted(size_t roiind);

protected:
	void resizeEvent(QResizeEvent *event) override;
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
	QSize getImageSize();

	MouseStatus mousestatus;
	ROIVert::ROISHAPE roishape = ROIVert::RECTANGLE;

	void pushROI();
	std::vector<std::unique_ptr<roi>> rois;
	size_t selroi = 0;

	cv::Mat roimap;
	void createROIMap();
	void updateROIMap(size_t roiind);

	QColor unselectedColor = QColor("#D90368");
	QColor selectedColor = QColor("#00CC66");
};

class Graphics_view_zoom : public QObject
{
	Q_OBJECT
public:
	Graphics_view_zoom(QGraphicsView *view);
	void gentle_zoom(double factor);
	void set_modifiers(Qt::KeyboardModifiers modifiers);
	void set_zoom_factor_base(double value);

private:
	QGraphicsView *_view;
	Qt::KeyboardModifiers _modifiers;
	double _zoom_factor_base;
	QPointF target_scene_pos, target_viewport_pos;
	bool eventFilter(QObject *object, QEvent *event);

signals:
	void zoomed();
};
