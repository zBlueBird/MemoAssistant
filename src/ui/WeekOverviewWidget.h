#pragma once

#include <QDate>
#include <QList>
#include <QMap>
#include <QWidget>

class QToolButton;

class WeekOverviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WeekOverviewWidget(QWidget *parent = nullptr);
    void setWeek(const QDate &startDate, const QDate &selectedDate, const QMap<QDate, int> &counts);

signals:
    void dateSelected(const QDate &date);

private:
    QList<QToolButton *> m_dayButtons;
    QList<QDate> m_dates;
};
