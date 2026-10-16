#pragma once

#include "models/DailyTaskSummary.h"

#include <QDate>
#include <QWidget>

class QLabel;

class WeekDayCard : public QWidget
{
    Q_OBJECT

public:
    explicit WeekDayCard(QWidget *parent = nullptr);
    void setDay(const QDate &date,
                const DailyTaskSummary &taskSummary,
                bool selected,
                bool today);

signals:
    void clicked();
    void doubleClicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void updateStyleState();

    QLabel *m_weekdayLabel;
    QLabel *m_dayLabel;
    QLabel *m_monthLabel;
    QLabel *m_countLabel;
    bool m_selected = false;
    bool m_today = false;
    bool m_hovered = false;
};
