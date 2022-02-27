#-------------------------------------------------
#
# Project created by QtCreator 2019-12-21T16:07:31
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Client
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    qcustomplot.cpp

HEADERS  += mainwidget.h \
    qcustomplot.h

FORMS    += mainwidget.ui

RESOURCES += \
    media/media.qrc
