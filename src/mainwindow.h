#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSettings>
#include <QModelIndex>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "ftpserver.h"
#include "configlist.h"
#include "dataconnection.h"


class FtpServer;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    const QString defSslKeyPath = ":/ssl/key2.pem";
    const QString defSslCertPath = ":/ssl/cert2.pem";
    const QString defSubnet = "192.168.1.0/24";
    const int defPort = 2121;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onNewPeerIp(const QString &ip);

signals:
    void sigQuit();

private:
    Ui::MainWindow *ui;
    QPoint winPos;
    QSize winSize;
    QSystemTrayIcon *trayIcon;
    QMenu* trayIconMenu;
    QString configPath;
    QString configFileName;
    ConfigList* confList = 0;
    QVector<ParamStruct> params;
    FtpServerParams servParams;
    QString sslKeyPath;
    QString sslCertPath;
    bool anonEnable = false;
    FtpServer *server = 0;

    void closeEvent(QCloseEvent *ev);
    void changeEvent(QEvent *ev);
    void setupParams();
    void setPublicParams(QSettings &set);
    void setParams(QVector<ParamStruct> &p);
    void onSettings();
    void onStart();
    void onStop();
    void onQuit();
    void onOk();
    void onCancel();
    void onConfDoubleClick(QModelIndex);
    void loadParams();
    void saveParams();
    void startServer();
};


#endif // MAINWINDOW_H
