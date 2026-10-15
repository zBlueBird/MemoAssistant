#include "ui/WeekOverviewWidget.h"

#include <QGridLayout>
#include <QLocale>
#include <QStyle>
#include <QToolButton>

WeekOverviewWidget::WeekOverviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("weekOverview"));
    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setHorizontalSpacing(7);
    layout->setVerticalSpacing(7);

    for (int index = 0; index < 7; ++index) {
        auto *button = new QToolButton(this);
        button->setObjectName(QStringLiteral("weekDayButton"));
        button->setCheckable(true);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setMinimumHeight(300);
        m_dayButtons.append(button);
        m_dates.append(QDate());
        layout->addWidget(button, 0, index);
        connect(button, &QToolButton::clicked, this, [this, index] {
            emit dateSelected(m_dates.at(index));
        });
    }
}

void WeekOverviewWidget::setWeek(const QDate &startDate,
                                 const QDate &selectedDate,
                                 const QMap<QDate, int> &counts)
{
    const QLocale locale;
    for (int index = 0; index < m_dayButtons.size(); ++index) {
        const QDate date = startDate.addDays(index);
        const int count = counts.value(date);
        m_dates[index] = date;
        m_dayButtons[index]->setText(tr("%1\n%2\n\n%3")
                                         .arg(locale.toString(date, QStringLiteral("ddd")),
                                              date.toString(QStringLiteral("M/d")),
                                              count == 0 ? tr("无任务") : tr("%1 项").arg(count)));
        m_dayButtons[index]->setChecked(date == selectedDate);
        m_dayButtons[index]->setProperty("hasTasks", count > 0);
        m_dayButtons[index]->style()->unpolish(m_dayButtons[index]);
        m_dayButtons[index]->style()->polish(m_dayButtons[index]);
    }
}
