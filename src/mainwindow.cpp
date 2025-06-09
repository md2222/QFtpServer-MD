#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ftpserver.h"
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QSslKey>
#include <QSslSocket>
#include <QDebug>

QString appName = "FTP server";


QString getSubStr(QString data, QString pref, QString post)
{
    int p1 = data.indexOf(pref);

    if (p1 >= 0)
    {
        p1 += pref.length();
        int p2 = data.indexOf(post, p1);
        if (p2 > p1)
        {
            return data.mid(p1, p2 - p1);
        }
    }

    return "";
}


QColor getThemeBgColor(QString qss)
{
    if (!qss.isEmpty())
    {
        QString colorName = getSubStr(qss, "background-color:", ";").trimmed();
        qDebug() << "getThemeBgColor:    colorName=" << colorName;

        if (!colorName.isEmpty())
            return QColor(colorName);
    }

    return QColor();
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //setLayout(ui->vl);
    ui->centralwidget->setLayout(ui->vl);

    //title = "Options";
    //setWindowTitle(title);
    setWindowIcon(QIcon(":/icons/ftp-48.png"));

    qInfo() << "QFtpServer-MD 0.0.2    3.06.2025";

    bool isQss = true;

    QColor themeBgColor = getThemeBgColor(qApp->styleSheet());
    if (!themeBgColor.isValid())
    {
        themeBgColor = palette().color(QPalette::Window);
        isQss = false;
    }
    //qDebug() << themeBgColor.name();

    if (!isQss)
        setStyleSheet("QDialogButtonBox { dialogbuttonbox-buttons-have-icons: 0; }");

    setProperty("qss", isQss);

    /*iconPref = ":/icons/";
    if (themeBgColor.lightness() < 130)
        iconPref = ":/icons/dark/";*/

    /*QVariant v = parent->property("qss");
    // If no such property exists, the returned variant is invalid
    if (v.isValid())
        setProperty("qss", v.toBool());*/

    QString appBaseName = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
    configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + appBaseName;
    qInfo() << "configDir=" << configPath;

    QDir configDir(configPath);
    if (!configDir.exists())
    {
        if (!configDir.mkpath(configPath))
        {
            QMessageBox::critical(this, appName, "Config directory create error.\n" + configPath, QMessageBox::Ok);
        }
    }

    configFileName = configPath + "/" + appBaseName + ".conf";
    qDebug() << "configFileName=" << configFileName;

    QAction* settsAction = new QAction(tr("Settings"), this);
    connect(settsAction, &QAction::triggered, this, &MainWindow::onSettings);

    QAction* startAction = new QAction(tr("Start"), this);
    connect(startAction, &QAction::triggered, this, &MainWindow::onStart);

    QAction* stopAction = new QAction(tr("Stop"), this);
    connect(stopAction, &QAction::triggered, this, &MainWindow::onStop);

    QAction* quitAction = new QAction(tr("Quit FTP Server"), this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuit);

    trayIconMenu = new QMenu();
    trayIconMenu->addAction(settsAction);
    trayIconMenu->addAction(startAction);
    trayIconMenu->addAction(stopAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    //setTrayIcon();
    trayIcon->setIcon(QIcon(":/icons/ftp-48-off.png"));

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onSettings);

    trayIcon->show();



    confList = new ConfigList(this);
    //conf->fileDialogSize = fileDialogSize;

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MainWindow::onOk);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &MainWindow::onCancel);

    ui->vl->addWidget(confList);
    ui->vl->addWidget(ui->buttonBox);

    connect(confList, &ConfigList::doubleClicked, this,  &MainWindow::onConfDoubleClick);

    subnet.second = 0;

    setupParams();

    //setParams(params);

    QSettings set(configFileName, QSettings::IniFormat);
    set.beginGroup("private");

    winPos = set.value("mainWinPos", QPoint(570, 280)).toPoint();
    winSize = set.value("mainWinSize", QSize(700, 400)).toSize();
    move(winPos);
    resize(winSize);
    //if (set.value("mainWinState").toInt() == Qt::WindowMaximized)
        //setWindowState(Qt::WindowMaximized);
    set.endGroup();
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *ev)
{
    // this keep window position
    ev->ignore();
    hide();

    //QMainWindow::closeEvent(ev);
}


void MainWindow::changeEvent(QEvent *ev)
{
    QMainWindow::changeEvent(ev);

    if(ev->type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
            this->hide();
    }
}


void MainWindow::setupParams()
{
    ParamStruct param;

    param.label = "Port for listen";  param.name = "port"; param.type = ConfigList::TypeNumber;
    param.defValue = defPort;  param.comment = "";  params.append(param);

    param.label = "Username";  param.name = "userName"; param.type = ConfigList::TypeString;
    param.defValue = "";  param.comment = "";  params.append(param);

    param.label = "Password";  param.name = "passw"; param.type = ConfigList::TypeString;
    param.defValue = "";  param.comment = "";  params.append(param);

    param.label = "Root path";  param.name = "rootPath"; param.type = ConfigList::TypeDir;
    param.defValue = "";  param.comment = "";  params.append(param);

    param.label = "Enable anonymous";  param.name = "anonEnable";  param.type = ConfigList::TypeBool;
    param.defValue = false;  param.comment = "";  params.append(param);

    param.label = "Read only";  param.name = "readOnly";  param.type = ConfigList::TypeBool;
    param.defValue = false;  param.comment = "";  params.append(param);

    param.label = "Only one IP";  param.name = "oneIp";  param.type = ConfigList::TypeBool;
    param.defValue = true;  param.comment = "";  params.append(param);

    param.label = "Data port range";  param.name = "portRange"; param.type = ConfigList::TypeString;
    param.defValue = "";  param.comment = "";  params.append(param);

    param.label = "Subnet";  param.name = "subnet"; param.type = ConfigList::TypeString;
    param.defValue = defSubnet;  param.comment = "CIDR notation";  params.append(param);

    param.label = "SSL key file";  param.name = "sslKeyPath";  param.type = ConfigList::TypeFile;
    param.defValue = defSslKeyPath;  param.comment = "You can/must make your own. See Openssl.";  params.append(param);

    param.label = "SSL certificate file";  param.name = "sslCertPath";  param.type = ConfigList::TypeFile;
    param.defValue = defSslCertPath;  param.comment = "";  params.append(param);
}


void MainWindow::setPublicParams(QSettings &set)
{
    port = set.value("port", defPort).toInt();
    userName = set.value("userName", "").toString();
    passw = set.value("passw", "").toString();
    rootPath = set.value("rootPath", "").toString();
    anonEnable = set.value("anonEnable", false).toBool();
    readOnly = set.value("readOnly", false).toBool();
    oneIp = set.value("oneIp", true).toBool();
    sslKeyPath = set.value("sslKeyPath", defSslKeyPath).toString();
    sslCertPath = set.value("sslCertPath", defSslCertPath).toString();

    QString s = set.value("portRange", "").toString();
    QTextStream ts(&s);  ts >> portRange.first >> portRange.second;
    //qDebug() << "MainWindow::setPublicParams:  portRange=" << portRange;

    s = set.value("subnet", defSubnet).toString();
    subnet = QHostAddress::parseSubnet(s);
}


void MainWindow::setParams(QVector<ParamStruct> &p)
{
    confList->clear();
    confList->addParams(p);
    confList->setFocus();
}


void MainWindow::onSettings()
{
    if (this->isVisible())
    {
        if (this->isActiveWindow())
            this->hide();
        else
            this->activateWindow();
    }
    else
    {
        loadParams();

        this->showNormal();
        this->activateWindow();
    }
}


void MainWindow::loadParams()
{
    QSettings set(configFileName, QSettings::IniFormat);
    set.beginGroup( "public" );

    ConfigList::loadValues(params, set);

    confList->clear();
    confList->addParams(params);
    confList->setFocus();
}


void MainWindow::saveParams()
{
    QSettings set(configFileName, QSettings::IniFormat);
    set.beginGroup( "public" );

    confList->saveValues(set);

    setPublicParams(set);  // new values in set
}


void MainWindow::onStart()
{
    server = 0;
    startServer();
}


void MainWindow::onStop()
{
    if (server)
    {
        server->close();
        server->deleteLater();
        qInfo() << "Stop listening";
    }

    trayIcon->setIcon(QIcon(":/icons/ftp-48-off.png"));
}


void MainWindow::onQuit()
{
    QSettings set(configFileName, QSettings::IniFormat);
    set.beginGroup("private");

    if (!this->isMaximized())
    {
        set.setValue("mainWinPos", pos());
        set.setValue("mainWinSize", size());
    }
    //set.setValue("mainWinState", (int)windowState());
    set.endGroup();

    onStop();

    qApp->quit();
    //emit sigQuit();
}


void MainWindow::onOk()
{
    QString err;

    while (1)
    {
        int n = confList->param("port")->value.toInt();
        if (n < 1024 || n > 65535)
        {
            err = "Port number must be between 1024 and 65535.";
            break;
        }

        QString s = confList->param("rootPath")->value.toString();
        QDir dir(s);
        if (!dir.exists())
        {
            err = "Directory not exists. [Root path]";
            break;
        }

        s = confList->param("portRange")->value.toString();
        int from = 0, to = 0;  QTextStream ts(&s);  ts >> from >> to;
        //qDebug() << "MainWindow::onOk:  portRange=" << from << to;
        if (from >= to || from < 1024 || from > 65535 || to < 1024 || to > 65535)
        {
            err = "Port range must be from min to max, port number between 1024 and 65535.";
            break;
        }

        s = confList->param("subnet")->value.toString();
        subnet = QHostAddress::parseSubnet(s);
        if (subnet.second == 0)
        {
            //error = "Parse subnet error.";
            err = "Parse subnet error:  " + s;
            break;
        }

        break;
    }

    if (!err.isEmpty())
    {
        QMessageBox::warning(this, appName, err, QMessageBox::Ok);
        return;
    }

    saveParams();

    hide();
}


void MainWindow::onCancel()
{
    hide();
}


void MainWindow::onConfDoubleClick(QModelIndex)
{
    confList->editCurrentRow(1);
}


void MainWindow::MainWindow::startServer()
{
    QSslConfiguration conf;

    conf.setLocalCertificateChain(QSslCertificate::fromPath(":/ssl/cert2.pem"));

    QFile file(":/ssl/key2.pem");
    file.open(QIODevice::ReadOnly);
    conf.setPrivateKey( QSslKey(file.readAll(), QSsl::Rsa, QSsl::Pem) );

    conf.setPeerVerifyMode(QSslSocket::VerifyNone);

    QSettings set(configFileName, QSettings::IniFormat);
    set.beginGroup( "public" );

    setPublicParams(set);

    if (server)
        delete server;

    server = new FtpServer(this, rootPath, port, userName, passw, readOnly, oneIp);
    server->setSslConf(conf);
    server->setPortRange(portRange);
    if (subnet.second)
        server->setSubnet(subnet);

    //QObject::connect(server, &FtpServer::newPeerIp, &MainWindow::onNewPeerIp);

    server->start();

    if (server->isListening())
    {
        //ui->statusBar->showMessage("Listening at " + FtpServer::lanIp());
        qInfo() << "Listening at " << FtpServer::lanIp();
        trayIcon->setIcon(QIcon(":/icons/ftp-48.png"));
    }
    else
    {
        //ui->statusBar->showMessage("Not listening");
        qInfo() << "Not listening";
    }
}


void MainWindow::onNewPeerIp(const QString &ip)
{
    qInfo() << "New peer IP:  " << ip;
}


