#include "ui/TaskCalendarWidget.h"

#include "ui/TaskCountStyle.h"

#include <QPainter>
#include <QTextCharFormat>

TaskCalendarWidget::TaskCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    setObjectName(QStringLiteral("taskCalendar"));
    setGridVisible(false);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);
    setNavigationBarVisible(false);
    setSelectionMode(QCalendarWidget::SingleSelection);

    QTextCharFormat weekendFormat;
    weekendFormat.setForeground(QColor(QStringLiteral("#4F7CFF")));
    setWeekdayTextFormat(Qt::Saturday, weekendFormat);
    setWeekdayTextFormat(Qt::Sunday, weekendFormat);
}

void TaskCalendarWidget::setTaskCounts(const QMap<QDate, DailyTaskSummary> &counts)
{
    m_taskCounts = counts;
    updateCells();
}

void TaskCalendarWidget::paintCell(QPainter *painter, const QRect &rect, QDate date) const
{
    QCalendarWidget::paintCell(painter, rect, date);

    const bool today = date == QDate::currentDate();
    const bool selected = date == selectedDate();
    if (today) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(QStringLiteral("#E5EDFF")));
        painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 7, 7);

        QFont dateFont = painter->font();
        dateFont.setBold(true);
        painter->setFont(dateFont);
        painter->setPen(QColor(QStringLiteral("#254FAF")));
        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));

        const QRect todayBadge(rect.left() + 4, rect.top() + 4, 18, 17);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(QStringLiteral("#315FD6")));
        painter->drawRoundedRect(todayBadge, 8, 8);
        QFont todayFont = painter->font();
        todayFont.setPixelSize(10);
        todayFont.setBold(true);
        painter->setFont(todayFont);
        painter->setPen(Qt::white);
        painter->drawText(todayBadge, Qt::AlignCenter, tr("今"));
        painter->restore();
    }

    if (selected) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(QColor(today ? QStringLiteral("#315FD6")
                                      : QStringLiteral("#7899ED")), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), 7, 7);
        painter->restore();
    }

    const DailyTaskSummary taskSummary = m_taskCounts.value(date);
    if (taskSummary.total <= 0) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    const QString text = taskSummary.total > 99
        ? QStringLiteral("99+")
        : QString::number(taskSummary.total);
    const int badgeWidth = taskSummary.total > 99 ? 28 : taskSummary.total > 9 ? 22 : 17;
    const QRect badge(rect.right() - badgeWidth - 3, rect.top() + 3, badgeWidth, 17);
    painter->setPen(Qt::NoPen);
    painter->setBrush(TaskCountStyle::badgeBackground(taskSummary));
    painter->drawRoundedRect(badge, 8, 8);
    QFont font = painter->font();
    font.setPixelSize(10);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(TaskCountStyle::badgeForeground(taskSummary));
    painter->drawText(badge, Qt::AlignCenter, text);
    painter->restore();
}
