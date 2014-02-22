#-------------------------------------------------
#
# Project created by QtCreator 2014-02-20T09:23:21
#
#-------------------------------------------------

QT       -= gui

TARGET = logging
TEMPLATE = lib

DEFINES += LOGGING_LIBRARY

SOURCES += logger.cpp \
    loggerengine.cpp \
    defaultformattingengine.cpp \
    richtextformattingengine.cpp \
    xmlformattingengine.cpp \
    htmlformattingengine.cpp \
    qtmsgformattingengine.cpp \
    fileloggerengine.cpp \
    qtmsgloggerengine.cpp \
    consoleloggerengine.cpp

HEADERS += logger.h\
        logging_global.h \
    formattingengine.h \
    loggerengine.h \
    loggerexportable.h \
    loggerfactory.h \
    defaultformattingengine.h \
    richtextformattingengine.h \
    xmlformattingengine.h \
    htmlformattingengine.h \
    qtmsgformattingengine.h \
    fileloggerengine.h \
    qtmsgloggerengine.h \
    consoleloggerengine.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
