#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <QObject>
#include <QSet>
#include <QSslConfiguration>
//#include <qftpserverlib_global.h>
//#include "dataconnection.h"


//class SslServer;
#include "sslserver.h"
#include "ftpcontrolconnection.h"


// The ftp server. Listens on a port, and starts a new control connection each
// time it gets connected.

class FtpServer : public QObject
{
    Q_OBJECT
public:
    explicit FtpServer(QObject *parent);
    ~FtpServer();
    bool setParams(FtpParams& params);

    bool start();
    // Whether or not the server is listening for incoming connections. If it
    // is not currently listening then there was an error - probably no
    // internet connection is available, or the port address might require root
    // priviledges (on Linux).
    bool isListening();
    QString errorString();
    void close();

    // Get the LAN IP of the host, e.g. "192.168.1.10".
    static QString lanIp();

signals:
    // A connection from a new IP has been established. This signal is emitted
    // when the FTP server is connected by a new IP. The new IP will then be
    // stored and will not cause this FTP server instance to emit this signal
    // any more.
    void newPeerIp(const QString &ip);
    void stop();

private slots:
    // Called by the SSL server when we have received a new connection.
    void startNewControlConnection();

private:
    FtpParams params;
    SslServer *server = 0;

    // All the IPs that this FTP server object has encountered in its lifetime.
    // See the signal newPeerIp.
    QSet<QString> encounteredIps;
};

#endif // FTPSERVER_H
