#-------------------------------------------------
#
# Project created by QtCreator 2014-02-27T19:28:05
#
#-------------------------------------------------

QT       += widgets xml

greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

TARGET = editor
TEMPLATE = lib

DEFINES += EDITOR_LIBRARY

SOURCES += codeeditor.cpp \
    codeeditorwidget.cpp

HEADERS += codeeditor.h\
        editor_global.h \
    codeeditorwidget.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lcore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/core.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/core.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../core/libcore.a
