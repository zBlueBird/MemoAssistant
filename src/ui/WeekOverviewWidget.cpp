#include "ui/WeekOverviewWidget.h"

#include "ui/WeekDayCard.h"

#include <QGridLayout>

WeekOverviewWidget::WeekOverviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("weekOverview"));
    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(8);

    for (int index = 0; index < 7; ++index) {
        auto *card = new WeekDayCard(this);
        m_dayCards.append(card);
        m_dates.append(QDate());
        layout->addWidget(card, index / 4, index % 4);
        connect(card, &WeekDayCard::clicked, this, [this, index] {
            emit dateSelected(m_dates.at(index));
        });
        connect(card, &WeekDayCard::doubleClicked, this, [this, index] {
            emit dateActivated(m_dates.at(index));
        });
    }
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 1);
    layout->setColumnStretch(3, 1);
}

void WeekOverviewWidget::setWeek(const QDate &startDate,
                                 const QDate &selectedDate,
                                 const QMap<QDate, DailyTaskSummary> &counts)
{
    const QDate today = QDate::currentDate();
    for (int index = 0; index < m_dayCards.size(); ++index) {
        const QDate date = startDate.addDays(index);
        m_dates[index] = date;
        m_dayCards[index]->setDay(date,
                                  counts.value(date),
                                  date == selectedDate,
                                  date == today);
    }
}
