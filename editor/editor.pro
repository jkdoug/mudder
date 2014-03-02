#-------------------------------------------------
#
# Project created by QtCreator 2014-02-27T19:28:05
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = editor
TEMPLATE = lib

DEFINES += EDITOR_LIBRARY

SOURCES += codeeditor.cpp \
    codeeditorwidget.cpp \
    codeeditorwindow.cpp \
    luahighlighter.cpp \
    xmlhighlighter.cpp \
    syntaxhighlighter.cpp \
    searchwidget.cpp

HEADERS += codeeditor.h\
        editor_global.h \
    codeeditorwidget.h \
    codeeditorwindow.h \
    luahighlighter.h \
    xmlhighlighter.h \
    syntaxhighlighter.h \
    searchwidget.h

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



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../logging/release/ -llogging
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../logging/debug/ -llogging
else:unix: LIBS += -L$$OUT_PWD/../logging/ -llogging

INCLUDEPATH += $$PWD/../logging
DEPENDPATH += $$PWD/../logging

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../logging/release/logging.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../logging/debug/logging.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../logging/liblogging.a

RESOURCES += \
    editor.qrc

FORMS += \
    searchwidget.ui
