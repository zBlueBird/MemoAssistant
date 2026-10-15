#pragma once

#include <QObject>
#include <QTimer>

class MemoRepository;
class QSystemTrayIcon;

class ReminderService : public QObject
{
    Q_OBJECT

public:
    ReminderService(MemoRepository *repository, QSystemTrayIcon *trayIcon, QObject *parent = nullptr);
    void start();

signals:
    void remindersTriggered();
    void reminderDue(const QString &message);

private slots:
    void checkDueReminders();

private:
    MemoRepository *m_repository;
    QSystemTrayIcon *m_trayIcon;
    QTimer m_timer;
};
