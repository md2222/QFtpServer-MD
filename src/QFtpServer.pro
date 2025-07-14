QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

CONFIG += c++11

TARGET = ftpserv

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


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    res.qrc


CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
