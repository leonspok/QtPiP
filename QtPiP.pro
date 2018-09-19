#-------------------------------------------------
#
# Project created by QtCreator 2018-09-14T21:56:47
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = QtPiP
TEMPLATE = app
INCLUDEPATH += .

NAME = QtPiP
DESCRIPTION = "Analog for Picture-in-Picture from macOS"
VERSION = 0.0.1
LICENSE = "MIT"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug
PKGCONFIG += mpv

SOURCES += \
    src/finishablesizegrip.cpp \
    src/main.cpp \
    src/pipwindow.cpp \
    src/videowidget.cpp \
    src/windowdragger.cpp

HEADERS += \
    src/finishablesizegrip.h \
    src/pipwindow.h \
    src/videowidget.h \
    src/windowdragger.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    main.qrc

unix {
    isEmpty(PREFIX):PREFIX = /usr #PREFIX - это путь установки
    BINDIR = $$PREFIX/bin # путь к бинарникам
    DATADIR = $$PREFIX/share # путь к различным компонентам приложения, от которых можно отказаться
    APPDIR = $$DATADIR/qtpip
    DEFINES += DATADIR=\"$$DATADIR\" \
        PKGDATADIR=\"$$PKGDATADIR\"

    # MAKE INSTALL
    INSTALLS += target \
        desktop \
        pixmaps
    target.path = $$BINDIR
    desktop.path = $$DATADIR/applications/
    desktop.files += $${TARGET}.desktop
    pixmaps.path = $$DATADIR/pixmaps/
    pixmaps.files += icons/qtpip.ico
#    icon22.path = $$DATADIR/icons/hicolor/22x22/apps/
#    icon22.files += images/$${TARGET}.png
}
