#pragma once

#include "models/Memo.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <optional>

class MemoRepository
{
public:
    explicit MemoRepository(QSqlDatabase database);

    std::optional<qint64> add(const Memo &memo);
    bool update(const Memo &memo);
    bool remove(qint64 id);
    bool setCompleted(qint64 id, bool completed);
    QList<Memo> createdOn(const QDate &localDate) const;
    QList<Memo> dueReminders(const QDateTime &utcNow) const;
    bool markReminderSent(qint64 id);
    std::optional<Memo> findById(qint64 id) const;
    QString lastError() const;

private:
    static Memo fromQuery(const class QSqlQuery &query);
    static QString toStorage(const QDateTime &dateTime);
    static QDateTime fromStorage(const QString &value);
    void setError(const QString &error) const;

    QSqlDatabase m_database;
    mutable QString m_lastError;
};
