#pragma once

#include <QPoint>
#include <QTimer>
#include <QWidget>

class FloatingBall : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingBall(QWidget *parent = nullptr);
    void moveToVisiblePosition(const QPoint &preferredPosition);
    void snapToNearestEdge();
    void setIncompleteCount(int count);
    void setAnimationEnabled(bool enabled);
    bool animationEnabled() const;

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
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QRect availableGeometryForPoint(const QPoint &point) const;
    QPoint boundedPosition(const QPoint &position, const QRect &availableGeometry) const;
    void updateAnimationTimer();

    QPoint m_pressGlobalPosition;
    QPoint m_windowPositionAtPress;
    bool m_dragging = false;
    bool m_hovered = false;
    bool m_animationEnabled = true;
    int m_incompleteCount = 0;
    qreal m_animationPhase = 0.0;
    QTimer m_animationTimer;
};
