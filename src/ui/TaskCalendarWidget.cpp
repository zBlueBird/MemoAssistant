#include "ui/TaskCalendarWidget.h"

#include <QPainter>

TaskCalendarWidget::TaskCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    setObjectName(QStringLiteral("taskCalendar"));
    setGridVisible(false);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
    setNavigationBarVisible(false);
    setSelectionMode(QCalendarWidget::SingleSelection);
}

void TaskCalendarWidget::setTaskCounts(const QMap<QDate, int> &counts)
{
    m_taskCounts = counts;
    updateCells();
}

void TaskCalendarWidget::paintCell(QPainter *painter, const QRect &rect, QDate date) const
{
    QCalendarWidget::paintCell(painter, rect, date);

    const int count = m_taskCounts.value(date);
    if (count <= 0) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    const QString text = count > 99 ? QStringLiteral("99+") : QString::number(count);
    const int badgeWidth = count > 9 ? 22 : 17;
    const QRect badge(rect.right() - badgeWidth - 3, rect.top() + 3, badgeWidth, 17);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(QStringLiteral("#4F7CFF")));
    painter->drawRoundedRect(badge, 8, 8);
    QFont font = painter->font();
    font.setPixelSize(10);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(badge, Qt::AlignCenter, text);
    painter->restore();
}
