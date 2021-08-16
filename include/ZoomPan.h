#pragma once
#include <QObject>

class QGraphicsView;

class ZoomPan : public QObject
{
	Q_OBJECT

public:
	ZoomPan(QGraphicsView* view);
	~ZoomPan();

	void Zoom(double factor);
	void setZoomFactor(double value) noexcept;

private:
	struct pimpl;
	std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
	bool eventFilter(QObject* object, QEvent* event) override;
};