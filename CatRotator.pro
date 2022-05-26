QT       += core gui
QT       += serialport
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogpreset.cpp \
    dialogrotator.cpp \
    dialogsetup.cpp \
    main.cpp \
    mainwindow.cpp \
    rotatordata.cpp \
    rotdaemon.cpp \
    rotudp.cpp

HEADERS += \
    dialogpreset.h \
    dialogrotator.h \
    dialogsetup.h \
    mainwindow.h \
    rotatordata.h \
    rotdaemon.h \
    rotudp.h

FORMS += \
    dialogpreset.ui \
    dialogrotator.ui \
    dialogsetup.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L$$PWD/hamlib/ -lhamlib
INCLUDEPATH += $$PWD/hamlib

VERSION = 1.0.0

RC_ICONS = catrotator.ico

QMAKE_LFLAGS += -no-pie
