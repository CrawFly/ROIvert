#pragma once
#include <QGraphicsView>

class ImageView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageView(QWidget* parent = nullptr);
    ~ImageView();
    void setImage(const QImage& image);
    QSize getImageSize() const noexcept;

signals:
    void mouseMoved(QPointF,bool);
    void mousePressed(QList<QGraphicsItem*>, const QPointF&, QMouseEvent*);
    void keyPressed(int key, Qt::KeyboardModifiers mod);
    void imageSizeUpdated(QSize);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};