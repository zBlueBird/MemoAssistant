#pragma once

#include <QLocalServer>
#include <QObject>
#include <QString>

class SingleInstanceGuard : public QObject
{
    Q_OBJECT

public:
    explicit SingleInstanceGuard(QString serverName, QObject *parent = nullptr);
    bool acquire();

signals:
    void activationRequested();

private slots:
    void handleConnection();

private:
    QString m_serverName;
    QLocalServer m_server;
};
