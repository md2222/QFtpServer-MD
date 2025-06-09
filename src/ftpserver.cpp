#include "ftpserver.h"
#include "ftpcontrolconnection.h"
#include "sslserver.h"

#include <QDebug>
#include <QNetworkInterface>
#include <QSslSocket>


FtpServer::FtpServer(QObject *parent, const QString &rootPath, int port, const QString &userName, const QString &password, bool readOnly, bool onlyOneIpAllowed) :
    QObject(parent)
{
    this->port = port;
    this->userName = userName;
    this->password = password;
    this->rootPath = rootPath;
    this->readOnly = readOnly;
    this->onlyOneIpAllowed = onlyOneIpAllowed;
}


bool FtpServer::setSslConf(QSslConfiguration &conf)
{
    //sslConf = conf;
    SslServer::setSslConf(conf);

    return true;
}


bool FtpServer::setPort(int port)
{
    this->port = port;
}


void FtpServer::setSubnet(const QPair<QHostAddress, int>& snet)
{
    if (snet.second)
        subnet = snet;
}


bool FtpServer::setPortRange(PortRange range)
{
    portRange = range;
    return true;
}


bool FtpServer::start()
{
    server = new SslServer(this);
    // In Qt4, QHostAddress::Any listens for IPv4 connections only, but as of
    // Qt5, it now listens on all available interfaces, and
    // QHostAddress::AnyIPv4 needs to be used if we want only IPv4 connections.
#if QT_VERSION >= 0x050000
    server->listen(QHostAddress::AnyIPv4, port);
#else
    server->listen(QHostAddress::Any, port);
#endif
    connect(server, SIGNAL(newConnection()), this, SLOT(startNewControlConnection()));
}


bool FtpServer::isListening()
{
    return server->isListening();
}


void FtpServer::close()
{
    if (server)
    {
        //server->
        //server->close();  //Segmentation fault
        server->deleteLater();
    }
}


QString FtpServer::lanIp()
{
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
            return address.toString();
        }
    }
    return "";
}

void FtpServer::startNewControlConnection()
{
    QSslSocket *socket = (QSslSocket *) server->nextPendingConnection();

    // If this is not a previously encountered IP emit the newPeerIp signal.
    QString peerIp = socket->peerAddress().toString();
    qDebug() << "connection from" << peerIp;

    if (subnet.second && !socket->peerAddress().isInSubnet(subnet))
    {
        qDebug() << "IP not in subnet:  " << peerIp;
        delete socket;
        return;
    }

    if (!encounteredIps.contains(peerIp)) {
        // If we don't allow more than one IP for the client, we close
        // that connection.
        if (onlyOneIpAllowed && !encounteredIps.isEmpty()) {
            delete socket;
            return;
        }

        emit newPeerIp(peerIp);
        encounteredIps.insert(peerIp);
    }

    // Create a new FTP control connection on this socket.
    new FtpControlConnection(this, socket, rootPath, userName, password, readOnly, portRange);
}
