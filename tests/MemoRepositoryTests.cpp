#include "data/DatabaseManager.h"
#include "data/MemoRepository.h"
#include "services/ReminderService.h"
#include "services/SettingsService.h"
#include "services/SingleInstanceGuard.h"

#include <QDateTime>
#include <QSettings>
#include <QSignalSpy>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <QTimeZone>
#include <QUuid>
#include <memory>

class MemoRepositoryTests : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void addAndFindMemo();
    void updateMemo();
    void completeMemo();
    void countIncompleteMemos();
    void returnDueRemindersOnly();
    void queryScheduledDateAndCounts();
    void migrateVersionOneDatabase();
    void reopenVersionTwoDatabase();
    void rejectUnsupportedDatabaseVersion();
    void triggerDueReminder();
    void persistSettings();
    void enforceSingleInstance();
    void deleteMemo();

private:
    std::unique_ptr<QTemporaryDir> m_directory;
    std::unique_ptr<DatabaseManager> m_database;
    std::unique_ptr<MemoRepository> m_repository;
};

void MemoRepositoryTests::init()
{
    m_directory = std::make_unique<QTemporaryDir>();
    QVERIFY(m_directory->isValid());
    m_database = std::make_unique<DatabaseManager>(m_directory->filePath(QStringLiteral("test.db")));
    QVERIFY2(m_database->initialize(), qPrintable(m_database->lastError()));
    m_repository = std::make_unique<MemoRepository>(m_database->database());
}

void MemoRepositoryTests::cleanup()
{
    m_repository.reset();
    m_database.reset();
    m_directory.reset();
}

void MemoRepositoryTests::addAndFindMemo()
{
    Memo memo;
    memo.title = QStringLiteral("测试备忘录");
    memo.content = QStringLiteral("详细内容");

    const auto id = m_repository->add(memo);
    QVERIFY2(id.has_value(), qPrintable(m_repository->lastError()));

    const auto saved = m_repository->findById(*id);
    QVERIFY(saved.has_value());
    QCOMPARE(saved->title, memo.title);
    QCOMPARE(saved->content, memo.content);
    QVERIFY(saved->createdAt.isValid());
    QCOMPARE(saved->scheduledDate, QDate::currentDate());
    QVERIFY(!saved->completed);
}

void MemoRepositoryTests::updateMemo()
{
    Memo memo;
    memo.title = QStringLiteral("修改前");
    const auto id = m_repository->add(memo);
    QVERIFY2(id.has_value(), qPrintable(m_repository->lastError()));

    memo = *m_repository->findById(*id);
    memo.title = QStringLiteral("修改后");
    memo.reminderAt = QDateTime::currentDateTimeUtc().addSecs(3600);
    QVERIFY(m_repository->update(memo));

    const auto saved = m_repository->findById(*id);
    QCOMPARE(saved->title, QStringLiteral("修改后"));
    QVERIFY(saved->reminderAt.isValid());
}

void MemoRepositoryTests::completeMemo()
{
    Memo memo;
    memo.title = QStringLiteral("要完成的事项");
    const auto id = m_repository->add(memo);
    QVERIFY2(id.has_value(), qPrintable(m_repository->lastError()));

    QVERIFY(m_repository->setCompleted(*id, true));
    const auto saved = m_repository->findById(*id);
    QVERIFY(saved->completed);
    QVERIFY(saved->completedAt.isValid());

    QVERIFY(m_repository->setCompleted(*id, false));
    const auto reopened = m_repository->findById(*id);
    QVERIFY(!reopened->completed);
    QVERIFY(!reopened->completedAt.isValid());
}

void MemoRepositoryTests::countIncompleteMemos()
{
    QCOMPARE(m_repository->incompleteCount(), 0);

    Memo first;
    first.title = QStringLiteral("未完成一");
    const auto firstId = m_repository->add(first);
    QVERIFY(firstId.has_value());

    Memo second;
    second.title = QStringLiteral("未完成二");
    const auto secondId = m_repository->add(second);
    QVERIFY(secondId.has_value());
    QCOMPARE(m_repository->incompleteCount(), 2);

    QVERIFY(m_repository->setCompleted(*firstId, true));
    QCOMPARE(m_repository->incompleteCount(), 1);
    QVERIFY(m_repository->remove(*secondId));
    QCOMPARE(m_repository->incompleteCount(), 0);
}

void MemoRepositoryTests::returnDueRemindersOnly()
{
    Memo due;
    due.title = QStringLiteral("已到期");
    due.reminderAt = QDateTime::currentDateTimeUtc().addSecs(-30);
    QVERIFY2(m_repository->add(due).has_value(), qPrintable(m_repository->lastError()));

    Memo future;
    future.title = QStringLiteral("未来提醒");
    future.reminderAt = QDateTime::currentDateTimeUtc().addSecs(3600);
    QVERIFY(m_repository->add(future).has_value());

    const QList<Memo> reminders = m_repository->dueReminders(QDateTime::currentDateTimeUtc());
    QCOMPARE(reminders.size(), 1);
    QCOMPARE(reminders.constFirst().title, due.title);
    QVERIFY(m_repository->markReminderSent(reminders.constFirst().id));
    QVERIFY(m_repository->dueReminders(QDateTime::currentDateTimeUtc()).isEmpty());
}

void MemoRepositoryTests::queryScheduledDateAndCounts()
{
    const QDate firstDate(2026, 10, 15);
    const QDate secondDate = firstDate.addDays(2);

    Memo first;
    first.title = QStringLiteral("第一项");
    first.scheduledDate = firstDate;
    QVERIFY(m_repository->add(first).has_value());

    Memo second;
    second.title = QStringLiteral("第二项");
    second.scheduledDate = secondDate;
    QVERIFY(m_repository->add(second).has_value());

    Memo third;
    third.title = QStringLiteral("第三项");
    third.scheduledDate = secondDate;
    QVERIFY(m_repository->add(third).has_value());

    const QList<Memo> scheduled = m_repository->scheduledOn(secondDate);
    QCOMPARE(scheduled.size(), 2);
    for (const Memo &memo : scheduled) {
        QCOMPARE(memo.scheduledDate, secondDate);
    }

    const QMap<QDate, int> counts = m_repository->taskCounts(firstDate, secondDate);
    QCOMPARE(counts.value(firstDate), 1);
    QCOMPARE(counts.value(secondDate), 2);
}

void MemoRepositoryTests::migrateVersionOneDatabase()
{
    m_repository.reset();
    m_database.reset();

    const QString path = m_directory->filePath(QStringLiteral("legacy.db"));
    const QString connectionName = QStringLiteral("legacy-%1")
                                       .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    {
        QSqlDatabase legacy = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        legacy.setDatabaseName(path);
        QVERIFY(legacy.open());
        QSqlQuery query(legacy);
        QVERIFY(query.exec(QStringLiteral(R"SQL(
            CREATE TABLE memos (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT NOT NULL,
                content TEXT NOT NULL DEFAULT '',
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL,
                reminder_at TEXT,
                completed_at TEXT,
                completed INTEGER NOT NULL DEFAULT 0,
                reminder_sent INTEGER NOT NULL DEFAULT 0
            )
        )SQL")));
        QVERIFY(query.exec(QStringLiteral(R"SQL(
            INSERT INTO memos(title, created_at, updated_at)
            VALUES('旧记录', '2026-10-14T16:30:00.000Z', '2026-10-14T16:30:00.000Z')
        )SQL")));
        QVERIFY(query.exec(QStringLiteral("PRAGMA user_version = 1")));
        legacy.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    m_database = std::make_unique<DatabaseManager>(path);
    QVERIFY2(m_database->initialize(), qPrintable(m_database->lastError()));
    m_repository = std::make_unique<MemoRepository>(m_database->database());

    QSqlQuery query(m_database->database());
    QVERIFY(query.exec(QStringLiteral("PRAGMA user_version")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 2);
    const auto migrated = m_repository->findById(1);
    QVERIFY(migrated.has_value());
    QVERIFY(migrated->scheduledDate.isValid());
}

void MemoRepositoryTests::reopenVersionTwoDatabase()
{
    m_repository.reset();
    m_database.reset();
    m_database = std::make_unique<DatabaseManager>(m_directory->filePath(QStringLiteral("test.db")));
    QVERIFY2(m_database->initialize(), qPrintable(m_database->lastError()));

    QSqlQuery query(m_database->database());
    QVERIFY(query.exec(QStringLiteral("PRAGMA user_version")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 2);
    m_repository = std::make_unique<MemoRepository>(m_database->database());
}

void MemoRepositoryTests::rejectUnsupportedDatabaseVersion()
{
    {
        QSqlQuery query(m_database->database());
        QVERIFY(query.exec(QStringLiteral("PRAGMA user_version = 3")));
    }

    m_repository.reset();
    m_database.reset();
    m_database = std::make_unique<DatabaseManager>(m_directory->filePath(QStringLiteral("test.db")));
    QVERIFY(!m_database->initialize());
    QVERIFY(m_database->lastError().contains(QStringLiteral("版本 3")));
}

void MemoRepositoryTests::triggerDueReminder()
{
    Memo memo;
    memo.title = QStringLiteral("测试提醒");
    memo.reminderAt = QDateTime::currentDateTimeUtc().addSecs(-1);
    const auto id = m_repository->add(memo);
    QVERIFY2(id.has_value(), qPrintable(m_repository->lastError()));

    ReminderService service(m_repository.get(), nullptr);
    QSignalSpy dueSpy(&service, &ReminderService::reminderDue);
    QSignalSpy triggeredSpy(&service, &ReminderService::remindersTriggered);

    QVERIFY(QMetaObject::invokeMethod(&service, "checkDueReminders", Qt::DirectConnection));
    QCOMPARE(dueSpy.count(), 1);
    QCOMPARE(dueSpy.constFirst().constFirst().toString(), memo.title);
    QCOMPARE(triggeredSpy.count(), 1);

    const auto saved = m_repository->findById(*id);
    QVERIFY(saved.has_value());
    QVERIFY(saved->reminderSent);

    QVERIFY(QMetaObject::invokeMethod(&service, "checkDueReminders", Qt::DirectConnection));
    QCOMPARE(dueSpy.count(), 1);
    QCOMPARE(triggeredSpy.count(), 1);
}

void MemoRepositoryTests::persistSettings()
{
    QSettings settings(QStringLiteral("MemoAssistant"), QStringLiteral("MemoAssistant"));
    const QString positionKey = QStringLiteral("floatingBall/position");
    const QVariant originalPosition = settings.value(positionKey);
    const bool hadOriginalPosition = settings.contains(positionKey);
    const QString animationKey = QStringLiteral("floatingBall/animationEnabled");
    const QVariant originalAnimation = settings.value(animationKey);
    const bool hadOriginalAnimation = settings.contains(animationKey);

    const QPoint expectedPosition(321, 654);
    {
        SettingsService service;
        service.setFloatingBallPosition(expectedPosition);
        service.setAnimationEnabled(false);
    }
    {
        SettingsService service;
        QCOMPARE(service.floatingBallPosition(), expectedPosition);
        QVERIFY(service.collapseDelayMs() > 0);
        QVERIFY(!service.animationEnabled());
    }

    if (hadOriginalPosition) {
        settings.setValue(positionKey, originalPosition);
    } else {
        settings.remove(positionKey);
    }
    if (hadOriginalAnimation) {
        settings.setValue(animationKey, originalAnimation);
    } else {
        settings.remove(animationKey);
    }
    settings.sync();
}

void MemoRepositoryTests::enforceSingleInstance()
{
    const QString serverName = QStringLiteral("MemoAssistantTests-%1")
                                   .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    SingleInstanceGuard first(serverName);
    QVERIFY(first.acquire());

    QSignalSpy activationSpy(&first, &SingleInstanceGuard::activationRequested);
    SingleInstanceGuard second(serverName);
    QVERIFY(!second.acquire());
    QTRY_COMPARE_WITH_TIMEOUT(activationSpy.count(), 1, 1'000);
}

void MemoRepositoryTests::deleteMemo()
{
    Memo memo;
    memo.title = QStringLiteral("待删除");
    const auto id = m_repository->add(memo);
    QVERIFY2(id.has_value(), qPrintable(m_repository->lastError()));

    QVERIFY(m_repository->remove(*id));
    QVERIFY(!m_repository->findById(*id).has_value());
}

QTEST_MAIN(MemoRepositoryTests)
#include "MemoRepositoryTests.moc"
