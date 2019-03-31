#-------------------------------------------------
#
# Project created by QtCreator 2016-05-31T16:31:32
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

TARGET = WordRepeater
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    ConfigForm.cpp \
    PlayForm.cpp \
    DbForm.cpp \
    MediaPlayer.cpp \
    ListForm.cpp \
    texts.cpp

HEADERS  += MainWindow.h \
    ConfigForm.h \
    PlayForm.h \
    PlayListItem.h \
    DbForm.h \
    MediaPlayer.h \
    Settings.h \
    ListForm.h \
    texts.h

FORMS    += MainWindow.ui \
    ConfigForm.ui \
    PlayForm.ui \
    DbForm.ui \
    ListForm.ui

RESOURCES += \
    resources.qrc

DISTFILES +=
