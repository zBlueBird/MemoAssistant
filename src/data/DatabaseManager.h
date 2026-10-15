#pragma once

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    explicit DatabaseManager(QString databasePath = {});
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    bool initialize();
    QSqlDatabase database() const;
    QString lastError() const;
    QString databasePath() const;

private:
    bool migrateSchema();
    bool migrateToVersion1(QSqlDatabase database);

    QString m_databasePath;
    QString m_connectionName;
    QString m_lastError;
};
