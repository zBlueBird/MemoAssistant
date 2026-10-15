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
    void returnDueRemindersOnly();
    void queryCreatedOnByLocalDate();
    void initializeExistingVersionOneDatabase();
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

void MemoRepositoryTests::queryCreatedOnByLocalDate()
{
    const QDate localDate(2026, 10, 15);
    const QTimeZone localZone = QTimeZone::systemTimeZone();

    Memo before;
    before.title = QStringLiteral("前一天");
    before.createdAt = QDateTime(localDate, QTime(0, 0), localZone).addMSecs(-1).toUTC();
    QVERIFY(m_repository->add(before).has_value());

    Memo start;
    start.title = QStringLiteral("当天开始");
    start.createdAt = QDateTime(localDate, QTime(0, 0), localZone).toUTC();
    QVERIFY(m_repository->add(start).has_value());

    Memo end;
    end.title = QStringLiteral("当天结束");
    end.createdAt = QDateTime(localDate.addDays(1), QTime(0, 0), localZone).addMSecs(-1).toUTC();
    QVERIFY(m_repository->add(end).has_value());

    Memo after;
    after.title = QStringLiteral("后一天");
    after.createdAt = QDateTime(localDate.addDays(1), QTime(0, 0), localZone).toUTC();
    QVERIFY(m_repository->add(after).has_value());

    const QList<Memo> result = m_repository->createdOn(localDate);
    QCOMPARE(result.size(), 2);
    QCOMPARE(result.at(0).title, end.title);
    QCOMPARE(result.at(1).title, start.title);
}

void MemoRepositoryTests::initializeExistingVersionOneDatabase()
{
    {
        QSqlQuery query(m_database->database());
        QVERIFY(query.exec(QStringLiteral("PRAGMA user_version")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
    }

    m_repository.reset();
    m_database.reset();
    m_database = std::make_unique<DatabaseManager>(m_directory->filePath(QStringLiteral("test.db")));
    QVERIFY2(m_database->initialize(), qPrintable(m_database->lastError()));

    {
        QSqlQuery query(m_database->database());
        QVERIFY(query.exec(QStringLiteral("PRAGMA user_version")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
    }
    m_repository = std::make_unique<MemoRepository>(m_database->database());
}

void MemoRepositoryTests::rejectUnsupportedDatabaseVersion()
{
    {
        QSqlQuery query(m_database->database());
        QVERIFY(query.exec(QStringLiteral("PRAGMA user_version = 2")));
    }

    m_repository.reset();
    m_database.reset();
    m_database = std::make_unique<DatabaseManager>(m_directory->filePath(QStringLiteral("test.db")));
    QVERIFY(!m_database->initialize());
    QVERIFY(m_database->lastError().contains(QStringLiteral("版本 2")));
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

    const QPoint expectedPosition(321, 654);
    {
        SettingsService service;
        service.setFloatingBallPosition(expectedPosition);
    }
    {
        SettingsService service;
        QCOMPARE(service.floatingBallPosition(), expectedPosition);
        QVERIFY(service.collapseDelayMs() > 0);
    }

    if (hadOriginalPosition) {
        settings.setValue(positionKey, originalPosition);
    } else {
        settings.remove(positionKey);
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
