#include "dataconnection.h"
#include "sslserver.h"
#include "ftpcommand.h"
#include <QSslSocket>
#include <QCoreApplication>


void delay(int msec)
{
    QTime t = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < t)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}



DataConnection::DataConnection(QObject *parent) :
    QObject(parent)
{
    server = new SslServer(this);
    connect(server, &SslServer::newConnection, this, &DataConnection::newConnection);
    socket = 0;
    isSocketReady = false;
    isWaitingForFtpCommand = false;
}


bool DataConnection::setPortRange(PortRange range)
{
    portRange = range;
    return true;
}


void DataConnection::scheduleConnectToHost(const QString &hostName, int port, bool encrypt)
{
    this->encrypt = encrypt;
    delete socket;
    this->hostName = hostName;
    this->port = port;
    isSocketReady = false;
    isWaitingForFtpCommand = true;
    isActiveConnection = true;
}


int DataConnection::listen(bool encrypt)
{
    qDebug() << "DataConnection::listen:  " << encrypt;

    this->encrypt = encrypt;

    delete socket;
    socket = 0;

    delete command;
    command = 0;

    isSocketReady = false;
    isWaitingForFtpCommand = true;
    isActiveConnection = false;

    server->close();

    int p = 0;

    if (portRange.first > 0)
    {
        srand(time(0));
        p = portRange.first + rand() % (portRange.second - portRange.first);
    }

    if (!server->listen(QHostAddress::Any, p))
        return 0;

    return server->serverPort();
}


bool DataConnection::setFtpCommand(FtpCommand *command)
{
    if (!isWaitingForFtpCommand)
        return false;
    isWaitingForFtpCommand = false;

    this->command = command;
    command->setParent(this);

    if (isActiveConnection)
    {
        socket = new QSslSocket(this);
        connect(socket, SIGNAL(connected()), SLOT(connected()));
        socket->connectToHost(hostName, port);
    }
    else
    {
        //if (isSocketReady) //md
            startFtpCommand();
    }

    return true;
}

FtpCommand *DataConnection::ftpCommand()
{
    if (isSocketReady)
    {
        return command;
    }

    return 0;
}

void DataConnection::newConnection()
{
    socket = (QSslSocket *)server->nextPendingConnection();
    qDebug() << "DataConnection::newConnection:  " << socket;
    server->close();

    if (encrypt)
    {
        connect(socket, &QSslSocket::errorOccurred, [](QAbstractSocket::SocketError error) {
                             qDebug() << "DataConnection::newConnection:  socket error:  " << error;
                         });
        //connect(socket, SIGNAL(encrypted()), this, SLOT(encrypted()));
        //connect(socket, &QSslSocket::encrypted, this, &DataConnection::encrypted); // 2025-06 not send or send later and fault
        //connect(socket, &QSslSocket::, this, &DataConnection::encrypted);
        SslServer::setLocalCertificateAndPrivateKey(socket);
        //socket->
        socket->startServerEncryption();
        qDebug() << "DataConnection::newConnection:  startServerEncryption  ";

        delay(1000);
        encrypted();  //md - signal not sended. this work
    }
    else
    {
        encrypted();
    }
}


void DataConnection::encrypted()
{
    qDebug() << "DataConnection::encrypted:  ";
    isSocketReady = true;
    startFtpCommand();
}


void DataConnection::connected()
{
    if (encrypt) {
        connect(socket, SIGNAL(encrypted()), this, SLOT(encrypted()));
        SslServer::setLocalCertificateAndPrivateKey(socket);
        socket->startServerEncryption();
    } else {
        encrypted();
    }
}


void DataConnection::startFtpCommand()
{
    qDebug() << "DataConnection::startFtpCommand:  " << command << isSocketReady;

    if (command && isSocketReady)
    {
        command->start(socket);
        socket = 0;
    }
}
