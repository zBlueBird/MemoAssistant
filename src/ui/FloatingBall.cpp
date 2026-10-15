#include "ui/FloatingBall.h"

#include <QContextMenuEvent>
#include <QEnterEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QShowEvent>
#include <QStyleHints>
#include <QtMath>
#include <numbers>

FloatingBall::FloatingBall(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    setObjectName(QStringLiteral("floatingBall"));
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedSize(58, 58);
    setToolTip(tr("点击查看备忘录，拖动可改变位置"));
    setCursor(Qt::PointingHandCursor);

    m_animationTimer.setInterval(125);
    m_animationTimer.setTimerType(Qt::CoarseTimer);
    connect(&m_animationTimer, &QTimer::timeout, this, [this] {
        m_animationPhase += m_hovered ? 0.9 : 0.45;
        if (m_animationPhase >= 2.0 * std::numbers::pi) {
            m_animationPhase -= 2.0 * std::numbers::pi;
        }
        update();
    });
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

void FloatingBall::setIncompleteCount(int count)
{
    const int normalizedCount = qMax(0, count);
    if (m_incompleteCount == normalizedCount) {
        return;
    }
    m_incompleteCount = normalizedCount;
    setToolTip(m_incompleteCount > 0
                   ? tr("%1 条未完成备忘录，点击查看").arg(m_incompleteCount)
                   : tr("没有未完成备忘录，点击查看"));
    update();
}

void FloatingBall::setAnimationEnabled(bool enabled)
{
    if (m_animationEnabled == enabled) {
        return;
    }
    m_animationEnabled = enabled;
    if (!enabled) {
        m_animationPhase = 0.0;
    }
    updateAnimationTimer();
    update();
}

bool FloatingBall::animationEnabled() const
{
    return m_animationEnabled;
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

    const qreal bob = m_animationEnabled ? qSin(m_animationPhase) * 1.3 : 0.0;
    const qreal wingPulse = m_animationEnabled
                                ? 0.65 + 0.35 * qAbs(qSin(m_animationPhase * 2.0))
                                : 0.8;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(28, 41, 61, 38));
    painter.drawEllipse(QRectF(13, 46, 34, 6));

    painter.save();
    painter.translate(0, bob);

    painter.setPen(QPen(QColor(QStringLiteral("#D5E8FF")), 1.2));
    painter.setBrush(QColor(235, 246, 255, 220));
    painter.save();
    painter.translate(25, 25);
    painter.rotate(-22 - wingPulse * 12);
    painter.drawEllipse(QRectF(-12, -11, 17, 11));
    painter.restore();
    painter.save();
    painter.translate(34, 25);
    painter.rotate(22 + wingPulse * 12);
    painter.drawEllipse(QRectF(-5, -11, 17, 11));
    painter.restore();

    painter.setPen(QPen(QColor(QStringLiteral("#513A17")), 1.6, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(20, 12, 12, 13), 70 * 16, 70 * 16);
    painter.drawArc(QRectF(27, 12, 12, 13), 40 * 16, 70 * 16);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#FFD54F")));
    painter.drawEllipse(QRectF(18, 21, 26, 25));

    painter.save();
    painter.setClipPath([] {
        QPainterPath path;
        path.addEllipse(QRectF(18, 21, 26, 25));
        return path;
    }());
    painter.setBrush(QColor(QStringLiteral("#4A3520")));
    painter.drawRect(QRectF(23, 20, 5, 28));
    painter.drawRect(QRectF(34, 20, 5, 28));
    painter.restore();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#3B2B1A")));
    painter.drawEllipse(QRectF(25, 28, 2.8, 3.4));
    painter.drawEllipse(QRectF(34.2, 28, 2.8, 3.4));
    painter.setPen(QPen(QColor(QStringLiteral("#3B2B1A")), 1.4, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(27.5, 31, 7, 6), 200 * 16, 140 * 16);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#4A3520")));
    QPolygonF sting;
    sting << QPointF(44, 33) << QPointF(49, 35.5) << QPointF(44, 38);
    painter.drawPolygon(sting);
    painter.restore();

    if (m_incompleteCount <= 0) {
        return;
    }

    const QString badgeText = m_incompleteCount > 99 ? QStringLiteral("99+")
                                                      : QString::number(m_incompleteCount);
    const qreal badgeWidth = m_incompleteCount > 99 ? 24.0 : m_incompleteCount > 9 ? 20.0 : 17.0;
    const QRectF badgeRect(width() - badgeWidth - 2.0, 2.0, badgeWidth, 17.0);
    painter.setPen(QPen(Qt::white, 1.5));
    painter.setBrush(QColor(QStringLiteral("#E5484D")));
    painter.drawRoundedRect(badgeRect, 8.5, 8.5);

    QFont badgeFont = painter.font();
    badgeFont.setFamily(QStringLiteral("Segoe UI"));
    badgeFont.setPixelSize(m_incompleteCount > 99 ? 9 : 10);
    badgeFont.setBold(true);
    painter.setFont(badgeFont);
    painter.setPen(Qt::white);
    painter.drawText(badgeRect, Qt::AlignCenter, badgeText);
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

void FloatingBall::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    updateAnimationTimer();
    QWidget::enterEvent(event);
}

void FloatingBall::leaveEvent(QEvent *event)
{
    m_hovered = false;
    updateAnimationTimer();
    QWidget::leaveEvent(event);
}

void FloatingBall::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateAnimationTimer();
}

void FloatingBall::hideEvent(QHideEvent *event)
{
    m_animationTimer.stop();
    QWidget::hideEvent(event);
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

void FloatingBall::updateAnimationTimer()
{
    if (!m_animationEnabled || !isVisible()) {
        m_animationTimer.stop();
        return;
    }
    m_animationTimer.setInterval(m_hovered ? 80 : 125);
    if (!m_animationTimer.isActive()) {
        m_animationTimer.start();
    }
}
