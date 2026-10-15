#include "ui/FloatingBall.h"

#include <QContextMenuEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QStyleHints>

FloatingBall::FloatingBall(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    setObjectName(QStringLiteral("floatingBall"));
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedSize(58, 58);
    setToolTip(tr("点击查看备忘录，拖动可改变位置"));
    setCursor(Qt::PointingHandCursor);
}

void FloatingBall::moveToVisiblePosition(const QPoint &preferredPosition)
{
    QScreen *screen = QGuiApplication::screenAt(preferredPosition + rect().center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect available = screen->availableGeometry();
    QPoint destination = preferredPosition;
    if (preferredPosition.isNull() || !available.adjusted(-width(), -height(), width(), height()).contains(preferredPosition)) {
        destination = QPoint(available.right() - width() - 18,
                             available.center().y() - height() / 2);
    }
    move(boundedPosition(destination, available));
}

void FloatingBall::snapToNearestEdge()
{
    const QRect available = availableGeometryForPoint(frameGeometry().center());
    const int left = available.left() + 8;
    const int right = available.right() - width() - 7;
    QPoint target = boundedPosition(pos(), available);
    target.setX((target.x() - left) < (right - target.x()) ? left : right);
    move(target);
    emit positionChanged(pos());
}

void FloatingBall::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF shadowRect = rect().adjusted(5, 6, -3, -2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(20, 38, 70, 45));
    painter.drawEllipse(shadowRect.translated(0, 2));

    QLinearGradient gradient(8, 5, width() - 5, height() - 3);
    gradient.setColorAt(0, QColor(QStringLiteral("#6D92FF")));
    gradient.setColorAt(1, QColor(QStringLiteral("#315FD6")));
    painter.setBrush(gradient);
    painter.drawEllipse(shadowRect);

    painter.setBrush(Qt::white);
    QPainterPath note;
    note.addRoundedRect(QRectF(20, 16, 23, 29), 4, 4);
    painter.drawPath(note);

    QPen linePen(QColor(QStringLiteral("#416DE0")), 2.4, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(linePen);
    painter.drawLine(QPointF(25, 25), QPointF(38, 25));
    painter.drawLine(QPointF(25, 32), QPointF(38, 32));
    painter.drawLine(QPointF(25, 39), QPointF(34, 39));
}

void FloatingBall::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressGlobalPosition = event->globalPosition().toPoint();
        m_windowPositionAtPress = pos();
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void FloatingBall::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        const QPoint delta = event->globalPosition().toPoint() - m_pressGlobalPosition;
        if (!m_dragging && delta.manhattanLength() >= QGuiApplication::styleHints()->startDragDistance()) {
            m_dragging = true;
            setCursor(Qt::ClosedHandCursor);
        }
        if (m_dragging) {
            const QPoint candidate = m_windowPositionAtPress + delta;
            move(boundedPosition(candidate, availableGeometryForPoint(event->globalPosition().toPoint())));
        }
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void FloatingBall::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::PointingHandCursor);
        if (m_dragging) {
            snapToNearestEdge();
        } else {
            emit clicked();
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void FloatingBall::contextMenuEvent(QContextMenuEvent *event)
{
    emit contextMenuRequested(event->globalPos());
    event->accept();
}

QRect FloatingBall::availableGeometryForPoint(const QPoint &point) const
{
    QScreen *screen = QGuiApplication::screenAt(point);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    return screen->availableGeometry();
}

QPoint FloatingBall::boundedPosition(const QPoint &position, const QRect &availableGeometry) const
{
    return QPoint(qBound(availableGeometry.left(), position.x(), availableGeometry.right() - width() + 1),
                  qBound(availableGeometry.top(), position.y(), availableGeometry.bottom() - height() + 1));
}
