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

VERSION = 1.5.1

# Windows
win32 {
    RC_ICONS = catrotator.ico
    QMAKE_TARGET_COPYRIGHT = IZ8EWD

    equals(QMAKE_HOST.arch, x86) {      #Win32
        message("Build Win32")
        DESTDIR = $$PWD/release/win32/CatRotator/
        LIBS += -L$$PWD/hamlib_w32/ -lhamlib
        INCLUDEPATH += $$PWD/hamlib_w32
    }

    equals(QMAKE_HOST.arch, x86_64) {   #Win64
        message("Build Win64")
        DESTDIR = $$PWD/release/win64/CatRotator/
        LIBS += -L$$PWD/hamlib/ -lhamlib
        INCLUDEPATH += $$PWD/hamlib
    }
}

# Linux
unix:!macx {
    message("Build Linux")
    LIBS += -L$$PWD/hamlib/ -lhamlib
    INCLUDEPATH += $$PWD/hamlib
    QMAKE_LFLAGS += -Wl,-rpath,\\$\$ORIGIN/hamlib/ #Set runtime shared libraries path to use local hamlib library
}

RESOURCES += qdarkstyle/dark/darkstyle.qrc

QMAKE_LFLAGS += -no-pie  #No Position Indipendent Executable
