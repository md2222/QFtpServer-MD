#include "ftpserver.h"
#include "ftpcontrolconnection.h"
#include "sslserver.h"

#include <QDebug>
#include <QNetworkInterface>
#include <QSslSocket>


FtpServer::FtpServer(QObject *parent) :
    QObject(parent)
{
}


FtpServer::~FtpServer()
{
    close();
}


bool FtpServer::setParams(FtpServerParams& params)
{
    this->params = params;

    SslServer::setSslConf(params.sslConf);

    return true;
}



bool FtpServer::start()
{
    qDebug() << "FtpServer::start:  " << params.port;
    //if (!server)
    {
        server = new SslServer(this);
        //connect(server, SIGNAL(newConnection()), this, SLOT(startNewControlConnection()));
        connect(server, &SslServer::newConnection, this, &FtpServer::startNewControlConnection);
    }

    return server->listen(QHostAddress::AnyIPv4, params.port);
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
        emit stop();
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

    if (params.subnet.second && !socket->peerAddress().isInSubnet(params.subnet))
    {
        qDebug() << "IP is not in the subnet:  " << peerIp;
        delete socket;
        return;
    }

    if (!encounteredIps.contains(peerIp))
    {
        // If we don't allow more than one IP for the client, we close
        // that connection.
        if (params.oneIp && !encounteredIps.isEmpty())
        {
            delete socket;
            return;
        }

        emit newPeerIp(peerIp);
        encounteredIps.insert(peerIp);
    }

    // Create a new FTP control connection on this socket.
    QObject* conn = new FtpControlConnection(this, socket, params.rootPath, params.userName, params.passw, params.readOnly, params.portRange);

    connect(this, &FtpServer::stop, conn, &FtpControlConnection::deleteLater);
}
