#-------------------------------------------------
#
# Project created by QtCreator 2019-12-20T15:05:20
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Server
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    qcustomplot.cpp \
    ledcontrol.cpp \
    adreader.cpp \
    buzzer.cpp

HEADERS  += mainwidget.h \
    qcustomplot.h \
    ledcontrol.h \
    adreader.h \
    buzzer.h

FORMS    += mainwidget.ui
