#pragma once

#include <QCalendarWidget>
#include <QMap>

class TaskCalendarWidget : public QCalendarWidget
{
    Q_OBJECT

public:
    explicit TaskCalendarWidget(QWidget *parent = nullptr);
    void setTaskCounts(const QMap<QDate, int> &counts);

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;

private:
    QMap<QDate, int> m_taskCounts;
};
