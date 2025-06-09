#include "mainwindow.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QResource>
#include <QFile>
#include <QObject>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setQuitOnLastWindowClosed(false);

    QString appName = app.applicationName();
    QString configDirPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + appName;

    QDir configDir(configDirPath);
    if (configDir.exists())
    {
        QString qssPath = ":/qss/app.qss";

        if ( !QResource::registerResource(configDirPath + "/" + appName + ".rcc") )
            qssPath = configDirPath + "/" + appName + ".qss";

        QFile file(qssPath);
        if (file.open(QFile::ReadOnly))
        {
            QString css = QLatin1String(file.readAll());
            file.close();
            app.setStyleSheet(css);
        }
    }

    MainWindow w;
    w.setWindowFlags( Qt::WindowTitleHint | Qt::WindowSystemMenuHint |  Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint );
    //QObject::connect(&w, &MainWindow::sigQuit, &app, QApplication::quit, Qt::QueuedConnection);
    //w.show();
    w.showMinimized();

    return app.exec();
}
