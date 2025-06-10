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

    subnet.second = 0;
}


FtpServer::~FtpServer()
{
    close();
}


bool FtpServer::setSslConf(QSslConfiguration &conf)
{
    //sslConf = conf;
    SslServer::setSslConf(conf);

    return true;
}


bool FtpServer::setPort(int p)
{
    this->port = p;
    return true;
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
    qDebug() << "FtpServer::start:  " << port;
    //if (!server)
    {
        server = new SslServer(this);
        //connect(server, SIGNAL(newConnection()), this, SLOT(startNewControlConnection()));
        connect(server, &SslServer::newConnection, this, &FtpServer::startNewControlConnection);
    }

    return server->listen(QHostAddress::AnyIPv4, port);
}


bool FtpServer::isListening()
{
    qDebug() << "FtpServer::isListening:  " << server;
    if (!server)
        return false;
    else
        return server->isListening();
}


QString FtpServer::errorString()
{
    if (!server)
        return QString("Server not created");
    else
        return server->errorString();
}


void FtpServer::close()
{
    if (server)
    {
        server->close();
        server->deleteLater();
        server = 0;
    }
}


QString FtpServer::lanIp()
{
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            return address.toString();
    }

    return "";
}


void FtpServer::startNewControlConnection()
{
    QSslSocket *socket = (QSslSocket *) server->nextPendingConnection();

    // If this is not a previously encountered IP emit the newPeerIp signal.
    QString peerIp = socket->peerAddress().toString();
    qDebug() << "New connection from" << peerIp;

    if (subnet.second && !socket->peerAddress().isInSubnet(subnet))
    {
        qDebug() << "IP is not in the subnet:  " << peerIp;
        delete socket;
        return;
    }

    if (!encounteredIps.contains(peerIp))
    {
        // If we don't allow more than one IP for the client, we close
        // that connection.
        if (onlyOneIpAllowed && !encounteredIps.isEmpty())
        {
            delete socket;
            return;
        }

        emit newPeerIp(peerIp);
        encounteredIps.insert(peerIp);
    }

    // Create a new FTP control connection on this socket.
    new FtpControlConnection(this, socket, rootPath, userName, password, readOnly, portRange);
}
