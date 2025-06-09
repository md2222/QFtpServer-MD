#include "sslserver.h"

#include <QSslSocket>
#include <QSslKey>

QSslConfiguration SslServer::sslConf;


SslServer::SslServer(QObject *parent) :
    QTcpServer(parent)
{
}


bool SslServer::setSslConf(QSslConfiguration &conf)
{
     SslServer::sslConf = conf;

    return true;
}

bool SslServer::setPort(int port)
{
    this->port = port;
}


void SslServer::setLocalCertificateAndPrivateKey(QSslSocket *socket)
{
/*    //socket->setPrivateKey(":/ssl/privkey.pem", QSsl::Rsa, QSsl::Pem, "39129380423984234012312");
    socket->setPrivateKey(":/ssl/key2.pem", QSsl::Rsa);
    Q_ASSERT(!socket->privateKey().isNull());
    //socket->setLocalCertificate(":/ssl/cacert.pem");
    socket->setLocalCertificate(":/ssl/cert2.pem");
    Q_ASSERT(!socket->localCertificate().isNull());

    socket->setPeerVerifyMode(QSslSocket::VerifyNone);
*/
    socket->setSslConfiguration(SslServer::sslConf);
}

void SslServer::incomingConnection(PortableSocketDescriptorType socketDescriptor)
{
    QSslSocket *socket = new QSslSocket(this);
    if (socket->setSocketDescriptor(socketDescriptor)) {
        addPendingConnection(socket);
    } else {
        delete socket;
    }
}
