#pragma once

#include <QPoint>
#include <QWidget>

class FloatingBall : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingBall(QWidget *parent = nullptr);
    void moveToVisiblePosition(const QPoint &preferredPosition);
    void snapToNearestEdge();

signals:
    void clicked();
    void positionChanged(const QPoint &position);
    void contextMenuRequested(const QPoint &globalPosition);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QRect availableGeometryForPoint(const QPoint &point) const;
    QPoint boundedPosition(const QPoint &position, const QRect &availableGeometry) const;

    QPoint m_pressGlobalPosition;
    QPoint m_windowPositionAtPress;
    bool m_dragging = false;
};
