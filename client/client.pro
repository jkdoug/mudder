#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T12:40:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    console.cpp \
    profile.cpp \
    consoledocument.cpp \
    consoledocumentlayout.cpp \
    profileitem.cpp \
    group.cpp \
    profileitemfactory.cpp \
    executable.cpp \
    matchable.cpp \
    alias.cpp \
    trigger.cpp \
    variable.cpp \
    event.cpp \
    accelerator.cpp \
    timer.cpp \
    commandline.cpp \
    consoledisplay.cpp

HEADERS  += mainwindow.h \
    console.h \
    profile.h \
    consoledocument.h \
    consoledocumentlayout.h \
    profileitem.h \
    group.h \
    profileitemfactory.h \
    xmlerror.h \
    executable.h \
    matchable.h \
    alias.h \
    trigger.h \
    variable.h \
    event.h \
    accelerator.h \
    timer.h \
    commandline.h \
    consoledisplay.h

FORMS    += mainwindow.ui \
    console.ui

RESOURCES += \
    client.qrc


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lua52/release/ -llua52
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lua52/debug/ -llua52
else:unix: LIBS += -L$$OUT_PWD/../lua52/ -llua52

INCLUDEPATH += $$PWD/../lua52/src
DEPENDPATH += $$PWD/../lua52/src

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lua52/release/lua52.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lua52/debug/lua52.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../lua52/liblua52.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lcore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/release/core.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../core/debug/core.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../core/libcore.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../editor/release/ -leditor
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../editor/debug/ -leditor
else:unix: LIBS += -L$$OUT_PWD/../editor/ -leditor

INCLUDEPATH += $$PWD/../editor
DEPENDPATH += $$PWD/../editor

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../editor/release/editor.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../editor/debug/editor.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../editor/libeditor.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../logging/release/ -llogging
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../logging/debug/ -llogging
else:unix: LIBS += -L$$OUT_PWD/../logging/ -llogging

INCLUDEPATH += $$PWD/../logging
DEPENDPATH += $$PWD/../logging
