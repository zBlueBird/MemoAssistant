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
    setFixedSize(68, 68);
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
    painter.setBrush(QColor(28, 41, 61, 30));
    painter.drawEllipse(QRectF(15, 55, 40, 6));

    painter.save();
    painter.translate(0, bob);

    // A round, side-facing mascot reads better at desktop-icon scale.
    painter.setPen(QPen(QColor(QStringLiteral("#B8DDF5")), 1.2));
    painter.setBrush(QColor(225, 244, 255, 225));
    painter.save();
    painter.translate(27, 29);
    painter.rotate(-24 - wingPulse * 14);
    painter.drawEllipse(QRectF(-15, -11, 21, 14));
    painter.restore();
    painter.save();
    painter.translate(40, 29);
    painter.rotate(24 + wingPulse * 14);
    painter.drawEllipse(QRectF(-6, -11, 21, 14));
    painter.restore();

    QPainterPath body;
    body.addEllipse(QRectF(18, 24, 34, 29));
    painter.setPen(QPen(QColor(QStringLiteral("#3E2C18")), 1.4));
    painter.setBrush(QColor(QStringLiteral("#FFCF3F")));
    painter.drawPath(body);

    painter.save();
    painter.setClipPath(body);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#3E2C18")));
    painter.drawRect(QRectF(27, 22, 6, 34));
    painter.drawRect(QRectF(41, 22, 6, 34));
    painter.restore();

    // Face sits on the left side; the yellow muzzle keeps it friendly.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#3E2C18")));
    painter.drawEllipse(QRectF(14, 26, 22, 23));
    painter.setBrush(QColor(QStringLiteral("#FFD95B")));
    painter.drawEllipse(QRectF(17, 29, 16, 17));
    painter.setBrush(QColor(QStringLiteral("#23180F")));
    painter.drawEllipse(QRectF(19.3, 33, 2.8, 3.4));
    painter.drawEllipse(QRectF(27.6, 33, 2.8, 3.4));
    painter.setPen(QPen(QColor(QStringLiteral("#3E2C18")), 1.25, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(21, 36, 7.5, 6), 200 * 16, 140 * 16);

    // Short antennae with rounded tips.
    painter.setPen(QPen(QColor(QStringLiteral("#3E2C18")), 1.7, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(20, 29), QPointF(16, 20));
    painter.drawLine(QPointF(28, 27), QPointF(31, 18));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#3E2C18")));
    painter.drawEllipse(QRectF(13.5, 17.5, 5, 5));
    painter.drawEllipse(QRectF(28.5, 15.5, 5, 5));

    // A small pointed tail completes the bee silhouette.
    QPolygonF sting;
    sting << QPointF(51, 34) << QPointF(59, 38.5) << QPointF(51, 43);
    painter.drawPolygon(sting);

    // Tiny feet add character without cluttering the face.
    painter.setPen(QPen(QColor(QStringLiteral("#3E2C18")), 1.6, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(28, 51), QPointF(25, 56));
    painter.drawLine(QPointF(39, 52), QPointF(42, 57));

    painter.restore();

    if (m_incompleteCount <= 0) {
        return;
    }

    const QString badgeText = m_incompleteCount > 99 ? QStringLiteral("99+")
                                                      : QString::number(m_incompleteCount);
    const qreal badgeWidth = m_incompleteCount > 99 ? 24.0 : m_incompleteCount > 9 ? 20.0 : 17.0;
    const QRectF badgeRect(width() - badgeWidth - 3.0, 2.0, badgeWidth, 17.0);
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
