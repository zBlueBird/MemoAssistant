#include "services/SingleInstanceGuard.h"

#include <QLocalSocket>
#include <utility>

SingleInstanceGuard::SingleInstanceGuard(QString serverName, QObject *parent)
    : QObject(parent), m_serverName(std::move(serverName))
{
    connect(&m_server, &QLocalServer::newConnection, this, &SingleInstanceGuard::handleConnection);
}

bool SingleInstanceGuard::acquire()
{
    QLocalSocket probe;
    probe.connectToServer(m_serverName);
    if (probe.waitForConnected(250)) {
        probe.write("activate");
        probe.flush();
        probe.waitForBytesWritten(250);
        probe.disconnectFromServer();
        return false;
    }

    QLocalServer::removeServer(m_serverName);
    return m_server.listen(m_serverName);
}

void SingleInstanceGuard::handleConnection()
{
    while (QLocalSocket *socket = m_server.nextPendingConnection()) {
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        socket->waitForReadyRead(100);
        socket->readAll();
        socket->disconnectFromServer();
        emit activationRequested();
    }
}
