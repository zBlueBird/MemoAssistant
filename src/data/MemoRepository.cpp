#include "data/MemoRepository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QTimeZone>
#include <QVariant>
#include <utility>

namespace {
constexpr auto selectColumns =
    "SELECT id, title, content, created_at, updated_at, reminder_at, completed_at, completed, reminder_sent, scheduled_date "
    "FROM memos ";
}

MemoRepository::MemoRepository(QSqlDatabase database)
    : m_database(std::move(database))
{
}

std::optional<qint64> MemoRepository::add(const Memo &memo)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QDateTime createdAt = memo.createdAt.isValid() ? memo.createdAt.toUTC() : now;

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(R"SQL(
        INSERT INTO memos(title, content, created_at, updated_at, reminder_at,
                          completed_at, completed, reminder_sent, scheduled_date)
        VALUES(:title, :content, :created_at, :updated_at, :reminder_at,
               :completed_at, :completed, :reminder_sent, :scheduled_date)
    )SQL"));
    query.bindValue(QStringLiteral(":title"), memo.title.trimmed());
    query.bindValue(QStringLiteral(":content"), memo.content.isNull() ? QStringLiteral("") : memo.content);
    query.bindValue(QStringLiteral(":created_at"), toStorage(createdAt));
    query.bindValue(QStringLiteral(":updated_at"), toStorage(now));
    query.bindValue(QStringLiteral(":reminder_at"),
                    memo.reminderAt.isValid() ? QVariant(toStorage(memo.reminderAt)) : QVariant());
    query.bindValue(QStringLiteral(":completed_at"),
                    memo.completedAt.isValid() ? QVariant(toStorage(memo.completedAt)) : QVariant());
    query.bindValue(QStringLiteral(":completed"), memo.completed);
    query.bindValue(QStringLiteral(":reminder_sent"), memo.reminderSent);
    const QDate scheduledDate = memo.scheduledDate.isValid()
                                    ? memo.scheduledDate
                                    : createdAt.toLocalTime().date();
    query.bindValue(QStringLiteral(":scheduled_date"), scheduledDate.toString(Qt::ISODate));

    if (!query.exec()) {
        setError(query.lastError().text());
        return std::nullopt;
    }
    return query.lastInsertId().toLongLong();
}

bool MemoRepository::update(const Memo &memo)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(R"SQL(
        UPDATE memos
        SET title = :title, content = :content, updated_at = :updated_at,
            reminder_at = :reminder_at, reminder_sent = :reminder_sent,
            scheduled_date = :scheduled_date
        WHERE id = :id
    )SQL"));
    query.bindValue(QStringLiteral(":title"), memo.title.trimmed());
    query.bindValue(QStringLiteral(":content"), memo.content.isNull() ? QStringLiteral("") : memo.content);
    query.bindValue(QStringLiteral(":updated_at"), toStorage(QDateTime::currentDateTimeUtc()));
    query.bindValue(QStringLiteral(":reminder_at"),
                    memo.reminderAt.isValid() ? QVariant(toStorage(memo.reminderAt)) : QVariant());
    query.bindValue(QStringLiteral(":reminder_sent"), memo.reminderSent);
    const QDate scheduledDate = memo.scheduledDate.isValid()
                                    ? memo.scheduledDate
                                    : memo.createdAt.toLocalTime().date();
    query.bindValue(QStringLiteral(":scheduled_date"), scheduledDate.toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":id"), memo.id);
    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        setError(QObject::tr("没有找到要更新的备忘录。"));
        return false;
    }
    return true;
}

bool MemoRepository::remove(qint64 id)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM memos WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        setError(QObject::tr("没有找到要删除的备忘录。"));
        return false;
    }
    return true;
}

bool MemoRepository::setCompleted(qint64 id, bool completed)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(R"SQL(
        UPDATE memos
        SET completed = :completed, completed_at = :completed_at, updated_at = :updated_at
        WHERE id = :id
    )SQL"));
    query.bindValue(QStringLiteral(":completed"), completed);
    query.bindValue(QStringLiteral(":completed_at"),
                    completed ? QVariant(toStorage(QDateTime::currentDateTimeUtc())) : QVariant());
    query.bindValue(QStringLiteral(":updated_at"), toStorage(QDateTime::currentDateTimeUtc()));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        setError(QObject::tr("没有找到要更新的备忘录。"));
        return false;
    }
    return true;
}

QList<Memo> MemoRepository::scheduledOn(const QDate &date) const
{
    QSqlQuery query(m_database);
    query.prepare(QString::fromLatin1(selectColumns)
                  + QStringLiteral("WHERE scheduled_date = :date "
                                   "ORDER BY completed ASC, created_at DESC"));
    query.bindValue(QStringLiteral(":date"), date.toString(Qt::ISODate));

    QList<Memo> result;
    if (!query.exec()) {
        setError(query.lastError().text());
        return result;
    }
    while (query.next()) {
        result.append(fromQuery(query));
    }
    return result;
}

QMap<QDate, DailyTaskSummary> MemoRepository::taskCounts(const QDate &startDate,
                                                          const QDate &endDate) const
{
    QMap<QDate, DailyTaskSummary> result;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(R"SQL(
        SELECT scheduled_date,
               COUNT(*),
               SUM(CASE WHEN completed = 0 THEN 1 ELSE 0 END)
        FROM memos
        WHERE scheduled_date >= :start AND scheduled_date <= :end
        GROUP BY scheduled_date
    )SQL"));
    query.bindValue(QStringLiteral(":start"), startDate.toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":end"), endDate.toString(Qt::ISODate));
    if (!query.exec()) {
        setError(query.lastError().text());
        return result;
    }
    while (query.next()) {
        result.insert(QDate::fromString(query.value(0).toString(), Qt::ISODate),
                      DailyTaskSummary{query.value(1).toInt(), query.value(2).toInt()});
    }
    return result;
}

int MemoRepository::incompleteCount() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM memos WHERE completed = 0"))
        || !query.next()) {
        setError(query.lastError().text());
        return 0;
    }
    return query.value(0).toInt();
}

QList<Memo> MemoRepository::dueReminders(const QDateTime &utcNow) const
{
    QSqlQuery query(m_database);
    query.prepare(QString::fromLatin1(selectColumns)
                  + QStringLiteral("WHERE reminder_at IS NOT NULL "
                                   "AND reminder_at <= :now AND reminder_sent = 0 AND completed = 0 "
                                   "ORDER BY reminder_at ASC"));
    query.bindValue(QStringLiteral(":now"), toStorage(utcNow));

    QList<Memo> result;
    if (!query.exec()) {
        setError(query.lastError().text());
        return result;
    }
    while (query.next()) {
        result.append(fromQuery(query));
    }
    return result;
}

bool MemoRepository::markReminderSent(qint64 id)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("UPDATE memos SET reminder_sent = 1 WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        setError(query.lastError().text());
        return false;
    }
    if (query.numRowsAffected() != 1) {
        setError(QObject::tr("没有找到要标记的备忘录。"));
        return false;
    }
    return true;
}

std::optional<Memo> MemoRepository::findById(qint64 id) const
{
    QSqlQuery query(m_database);
    query.prepare(QString::fromLatin1(selectColumns) + QStringLiteral("WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        setError(query.lastError().text());
        return std::nullopt;
    }
    if (!query.next()) {
        return std::nullopt;
    }
    return fromQuery(query);
}

QString MemoRepository::lastError() const
{
    return m_lastError;
}

Memo MemoRepository::fromQuery(const QSqlQuery &query)
{
    Memo memo;
    memo.id = query.value(0).toLongLong();
    memo.title = query.value(1).toString();
    memo.content = query.value(2).toString();
    memo.createdAt = fromStorage(query.value(3).toString());
    memo.updatedAt = fromStorage(query.value(4).toString());
    memo.reminderAt = fromStorage(query.value(5).toString());
    memo.completedAt = fromStorage(query.value(6).toString());
    memo.completed = query.value(7).toBool();
    memo.reminderSent = query.value(8).toBool();
    memo.scheduledDate = QDate::fromString(query.value(9).toString(), Qt::ISODate);
    return memo;
}

QString MemoRepository::toStorage(const QDateTime &dateTime)
{
    return dateTime.toUTC().toString(Qt::ISODateWithMs);
}

QDateTime MemoRepository::fromStorage(const QString &value)
{
    return value.isEmpty() ? QDateTime() : QDateTime::fromString(value, Qt::ISODateWithMs).toUTC();
}

void MemoRepository::setError(const QString &error) const
{
    m_lastError = error;
}
