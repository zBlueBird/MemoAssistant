#include "ui/WeekDayCard.h"

#include <QEnterEvent>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QStyle>
#include <QVBoxLayout>

WeekDayCard::WeekDayCard(QWidget *parent)
    : QWidget(parent),
      m_weekdayLabel(new QLabel(this)),
      m_dayLabel(new QLabel(this)),
      m_monthLabel(new QLabel(this)),
      m_countLabel(new QLabel(this))
{
    setObjectName(QStringLiteral("weekDayCard"));
    setAttribute(Qt::WA_StyledBackground);
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(132);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_weekdayLabel->setObjectName(QStringLiteral("weekCardWeekday"));
    m_dayLabel->setObjectName(QStringLiteral("weekCardDay"));
    m_monthLabel->setObjectName(QStringLiteral("weekCardMonth"));
    m_countLabel->setObjectName(QStringLiteral("weekCardCount"));
    for (QLabel *label : {m_weekdayLabel, m_dayLabel, m_monthLabel, m_countLabel}) {
        label->setAlignment(Qt::AlignCenter);
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 10, 5, 10);
    layout->setSpacing(2);
    layout->addWidget(m_weekdayLabel);
    layout->addWidget(m_dayLabel);
    layout->addWidget(m_monthLabel);
    layout->addStretch();
    layout->addWidget(m_countLabel, 0, Qt::AlignHCenter);
}

void WeekDayCard::setDay(const QDate &date, int taskCount, bool selected, bool today)
{
    m_selected = selected;
    m_today = today;
    m_weekdayLabel->setText(QLocale().toString(date, QStringLiteral("ddd")));
    m_dayLabel->setText(QString::number(date.day()));
    m_monthLabel->setText(date.toString(QStringLiteral("M月")));
    m_countLabel->setText(taskCount == 0 ? tr("暂无") : tr("%1 项").arg(taskCount));
    m_countLabel->setProperty("hasTasks", taskCount > 0);
    updateStyleState();
}

void WeekDayCard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->position().toPoint())) {
        emit clicked();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void WeekDayCard::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    updateStyleState();
    QWidget::enterEvent(event);
}

void WeekDayCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    updateStyleState();
    QWidget::leaveEvent(event);
}

void WeekDayCard::updateStyleState()
{
    setProperty("selected", m_selected);
    setProperty("today", m_today);
    setProperty("hovered", m_hovered);
    style()->unpolish(this);
    style()->polish(this);
    m_countLabel->style()->unpolish(m_countLabel);
    m_countLabel->style()->polish(m_countLabel);
    update();
}
