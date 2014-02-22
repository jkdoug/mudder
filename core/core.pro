#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T13:12:41
#
#-------------------------------------------------

QT += gui widgets

TARGET = core
TEMPLATE = lib

DEFINES += CORE_LIBRARY

SOURCES += \
    coresettings.cpp \
    coreapplication.cpp

HEADERS +=\
        core_global.h \
    coresettings.h \
    coreapplication.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
