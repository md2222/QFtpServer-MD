#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslKey>


//class QSslSocket;


// A simple SSL server. Returns a QSslSocket instead of a QTcpSocket, but
// started in unencrypted mode, so it can be used as a regular QTcpServer, as
// well. The QSslSockets are loaded with a default certificate coming from the
// resource files.

//typedef qintptr PortableSocketDescriptorType;

class SslServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit SslServer(QObject *parent);
    bool setPort(int p);

    // Sets the local certificate and private key for the socket, so
    // startServerEncryption() can be used.
    static bool setSslConf(QSslConfiguration &conf);
    static void setLocalCertificateAndPrivateKey(QSslSocket *socket);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    int port = 0;
    //static QSslConfiguration sslConf;
};

extern QSslConfiguration sslConf;

#endif // SSLSERVER_H
