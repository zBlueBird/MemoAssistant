#include "data/DatabaseManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <utility>

DatabaseManager::DatabaseManager(QString databasePath)
    : m_databasePath(std::move(databasePath)),
      m_connectionName(QStringLiteral("memo-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)))
{
    if (m_databasePath.isEmpty()) {
        const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        m_databasePath = QDir(dataDirectory).filePath(QStringLiteral("memo.db"));
    }
}

DatabaseManager::~DatabaseManager()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        {
            QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
            db.close();
        }
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DatabaseManager::initialize()
{
    const QFileInfo databaseInfo(m_databasePath);
    if (!QDir().mkpath(databaseInfo.absolutePath())) {
        m_lastError = QObject::tr("无法创建数据目录：%1").arg(databaseInfo.absolutePath());
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(m_databasePath);
    if (!db.open()) {
        m_lastError = db.lastError().text();
        return false;
    }

    QSqlQuery pragma(db);
    if (!pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"))
        || !pragma.exec(QStringLiteral("PRAGMA journal_mode = WAL"))
        || !pragma.exec(QStringLiteral("PRAGMA busy_timeout = 5000"))) {
        m_lastError = pragma.lastError().text();
        return false;
    }

    return migrateSchema();
}

bool DatabaseManager::migrateSchema()
{
    constexpr int latestSchemaVersion = 1;

    QSqlDatabase db = database();
    QSqlQuery versionQuery(db);
    if (!versionQuery.exec(QStringLiteral("PRAGMA user_version")) || !versionQuery.next()) {
        m_lastError = versionQuery.lastError().text();
        return false;
    }

    const int currentVersion = versionQuery.value(0).toInt();
    if (currentVersion > latestSchemaVersion) {
        m_lastError = QObject::tr("数据库版本 %1 高于当前程序支持的版本 %2。")
                          .arg(currentVersion)
                          .arg(latestSchemaVersion);
        return false;
    }

    if (currentVersion < 1 && !migrateToVersion1(db)) {
        return false;
    }

    return true;
}

bool DatabaseManager::migrateToVersion1(QSqlDatabase db)
{
    if (!db.transaction()) {
        m_lastError = db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    const bool ok = query.exec(QStringLiteral(R"SQL(
        CREATE TABLE memos (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL CHECK(length(trim(title)) > 0),
            content TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            reminder_at TEXT,
            completed_at TEXT,
            completed INTEGER NOT NULL DEFAULT 0 CHECK(completed IN (0, 1)),
            reminder_sent INTEGER NOT NULL DEFAULT 0 CHECK(reminder_sent IN (0, 1))
        )
    )SQL"))
        && query.exec(QStringLiteral(
            "CREATE INDEX idx_memos_created_at ON memos(created_at)"))
        && query.exec(QStringLiteral(
            "CREATE INDEX idx_memos_due ON memos(reminder_at, reminder_sent, completed)"))
        && query.exec(QStringLiteral("PRAGMA user_version = 1"));

    if (!ok) {
        m_lastError = query.lastError().text();
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        m_lastError = db.lastError().text();
        return false;
    }
    return true;
}

QSqlDatabase DatabaseManager::database() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

QString DatabaseManager::databasePath() const
{
    return m_databasePath;
}
