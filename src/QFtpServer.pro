# Add files and directories to ship with the application 
# by adapting the examples below.
# file1.source = myfile
# dir1.source = mydir
#DEPLOYMENTFOLDERS = # file1 dir1

#symbian:TARGET.UID3 = 0xE6728FD6

# Smart Installer package's UID
# This UID is from the protected range 
# and therefore the package will fail to install if self-signed
# By default qmake uses the unprotected range value if unprotected UID is defined for the application
# and 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
#symbian:TARGET.CAPABILITY += NetworkServices

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the 
# MOBILITY variable. 
# CONFIG += mobility
# MOBILITY +=

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = ftpserv

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp mainwindow.cpp \
    configlist.cpp \
    dataconnection.cpp \
    ftpcommand.cpp \
    ftpcontrolconnection.cpp \
    ftplistcommand.cpp \
    ftpretrcommand.cpp \
    ftpserver.cpp \
    ftpstorcommand.cpp \
    sslserver.cpp
HEADERS += mainwindow.h \
    configlist.h \
    dataconnection.h \
    ftpcommand.h \
    ftpcontrolconnection.h \
    ftplistcommand.h \
    ftpretrcommand.h \
    ftpserver.h \
    ftpstorcommand.h \
    qftpserverlib_global.h \
    sslserver.h
FORMS += mainwindow.ui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Please do not modify the following two lines. Required for deployment.
#include(deployment.pri)
#qtcAddDeployment()

RESOURCES += \
    res.qrc \
    res.qrc

#OTHER_FILES += \
#    cacert.pem \
#    android/AndroidManifest.xml

#ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

DISTFILES += \
    QFtpServer.desktop \
    QFtpServer.pro.user \
    QFtpServer.svg \
    QFtpServer64.png \
    QFtpServer80.png \
    QFtpServer_harmattan.desktop \
    deployment.pri

#win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QFtpServerLib/release/ -lQFtpServerLib
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QFtpServerLib/debug/ -lQFtpServerLib
#else:unix: LIBS += -L$$OUT_PWD/../QFtpServerLib/ -lQFtpServerLib


#INCLUDEPATH += $$PWD/../QFtpServerLib
#DEPENDPATH += $$PWD/../QFtpServerLib

#SUBDIRS += QFtpServerLib.pro
