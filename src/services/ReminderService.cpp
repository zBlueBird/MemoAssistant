#include "services/ReminderService.h"

#include "data/MemoRepository.h"

#include <QApplication>
#include <QDateTime>
#include <QSystemTrayIcon>

ReminderService::ReminderService(MemoRepository *repository, QSystemTrayIcon *trayIcon, QObject *parent)
    : QObject(parent), m_repository(repository), m_trayIcon(trayIcon)
{
    m_timer.setInterval(30'000);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &ReminderService::checkDueReminders);
}

void ReminderService::start()
{
    checkDueReminders();
    m_timer.start();
}

void ReminderService::checkDueReminders()
{
    const QList<Memo> due = m_repository->dueReminders(QDateTime::currentDateTimeUtc());
    if (due.isEmpty()) {
        return;
    }

    const QString message = due.size() == 1
                                ? due.constFirst().title
                                : tr("你有 %1 条备忘录已到提醒时间，点击查看。").arg(due.size());

    if (m_trayIcon && QSystemTrayIcon::isSystemTrayAvailable() && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(tr("备忘录提醒"),
                                message,
                                QSystemTrayIcon::Information,
                                15'000);
    }
    QApplication::alert(QApplication::activeWindow(), 8'000);
    emit reminderDue(message);

    bool changed = false;
    for (const Memo &memo : due) {
        changed = m_repository->markReminderSent(memo.id) || changed;
    }
    if (changed) {
        emit remindersTriggered();
    }
}
