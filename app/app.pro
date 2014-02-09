#-------------------------------------------------
#
# Project created by QtCreator 2013-04-27T21:29:30
#
#-------------------------------------------------

QT       += core gui network xml xmlpatterns multimedia printsupport sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mudder
TEMPLATE = app

QTPLUGIN += qsqlite

SOURCES += main.cpp\
        mainwindow.cpp \
    profile.cpp \
    connection.cpp \
    console.cpp \
    splitterhandle.cpp \
    splitter.cpp \
    commandline.cpp \
    engine.cpp \
    dialogconnect.cpp \
    group.cpp \
    executable.cpp \
    matchable.cpp \
    alias.cpp \
    trigger.cpp \
    timer.cpp \
    variable.cpp \
    dialogprofile.cpp \
    dialogsettings.cpp \
    accelerator.cpp \
    profileitem.cpp \
    dialogtrigger.cpp \
    dialogalias.cpp \
    dialogaccelerator.cpp \
    dialogtimer.cpp \
    dialogvariable.cpp \
    dialoggroup.cpp \
    event.cpp \
    luahighlighter.cpp \
    hyperlink.cpp \
    maproom.cpp \
    maparea.cpp \
    mapexit.cpp \
    mapenvironment.cpp \
    mapwidget.cpp \
    mapengine.cpp \
    consoledisplay.cpp \
    textdocument.cpp \
    textblockdata.cpp \
    consoledocumentlayout.cpp \
    dialogglobal.cpp \
    options.cpp \
    codeeditor.cpp \
    codeeditorwidget.cpp \
    codeeditorwindow.cpp \
    xmlhighlighter.cpp \
    syntaxhighlighter.cpp \
    configwidget.cpp \
    configconnection.cpp \
    configdisplay.cpp \
    configlogging.cpp \
    configscripting.cpp \
    configeditor.cpp

HEADERS  += mainwindow.h \
    profile.h \
    connection.h \
    console.h \
    splitterhandle.h \
    splitter.h \
    commandline.h \
    engine.h \
    dialogconnect.h \
    group.h \
    executable.h \
    matchable.h \
    alias.h \
    trigger.h \
    timer.h \
    variable.h \
    dialogprofile.h \
    dialogsettings.h \
    accelerator.h \
    profileitem.h \
    dialogtrigger.h \
    dialogalias.h \
    dialogaccelerator.h \
    dialogtimer.h \
    dialogvariable.h \
    dialoggroup.h \
    xmlexception.h \
    event.h \
    luawrapper.h \
    luawrapperutils.h \
    luahighlighter.h \
    hyperlink.h \
    maproom.h \
    maparea.h \
    mapexit.h \
    mapenvironment.h \
    mapwidget.h \
    mapengine.h \
    consoledisplay.h \
    textdocument.h \
    textblockdata.h \
    consoledocumentlayout.h \
    dialogglobal.h \
    options.h \
    codeeditor.h \
    codeeditorwidget.h \
    codeeditorwindow.h \
    xmlhighlighter.h \
    syntaxhighlighter.h \
    configwidget.h \
    configconnection.h \
    configdisplay.h \
    configlogging.h \
    configscripting.h \
    configeditor.h

FORMS    += mainwindow.ui \
    console.ui \
    dialogconnect.ui \
    dialogprofile.ui \
    dialogsettings.ui \
    dialogtrigger.ui \
    dialogalias.ui \
    dialogaccelerator.ui \
    dialogtimer.ui \
    dialogvariable.ui \
    dialoggroup.ui \
    dialogglobal.ui \
    configwidget.ui \
    configconnection.ui \
    configdisplay.ui \
    configlogging.ui \
    configscripting.ui \
    configeditor.ui

RESOURCES += \
    mudder.qrc


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lua52/release/ -llua52
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lua52/debug/ -llua52
else:unix: LIBS += -L$$OUT_PWD/../lua52/ -llua52

INCLUDEPATH += $$PWD/../lua52
DEPENDPATH += $$PWD/../lua52

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lua52/release/lua52.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lua52/debug/lua52.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../lua52/liblua52.a
