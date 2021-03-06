#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T12:40:52
#
#-------------------------------------------------

QT       += core gui network xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mudder
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
    consoledisplay.cpp \
    engine.cpp \
    settingswindow.cpp \
    editvariable.cpp \
    editsetting.cpp \
    editaccelerator.cpp \
    editalias.cpp \
    edittimer.cpp \
    editgroup.cpp \
    editevent.cpp \
    edittrigger.cpp \
    logging.cpp \
    configconnection.cpp \
    configscripting.cpp \
    dialogprofile.cpp \
    configinput.cpp \
    configoutput.cpp \
    settingsfiltermodel.cpp \
    richtextdelegate.cpp \
    luastate.cpp

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
    consoledisplay.h \
    engine.h \
    settingswindow.h \
    editvariable.h \
    editsetting.h \
    editaccelerator.h \
    editalias.h \
    edittimer.h \
    editgroup.h \
    editevent.h \
    edittrigger.h \
    ../luabridge/Source/LuaBridge/LuaBridge.h \
    ../luabridge/Source/LuaBridge/RefCountedObject.h \
    ../luabridge/Source/LuaBridge/RefCountedPtr.h \
    logging.h \
    configconnection.h \
    configscripting.h \
    dialogprofile.h \
    configinput.h \
    configoutput.h \
    settingsfiltermodel.h \
    richtextdelegate.h \
    luastate.h

FORMS    += mainwindow.ui \
    console.ui \
    settingswindow.ui \
    editvariable.ui \
    editaccelerator.ui \
    editalias.ui \
    edittimer.ui \
    editgroup.ui \
    editevent.ui \
    edittrigger.ui \
    configconnection.ui \
    configscripting.ui \
    dialogprofile.ui \
    configinput.ui \
    configoutput.ui

RESOURCES += \
    client.qrc

win32-g++: QMAKE_CXXFLAGS += -Wno-ignored-qualifiers

INCLUDEPATH += $$PWD/../luabridge/Source/LuaBridge

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lua52/release/ -llua52
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lua52/debug/ -llua52
else:unix: LIBS += -L$$OUT_PWD/../lua52/ -llua52

INCLUDEPATH += $$PWD/../lua52/src
DEPENDPATH += $$PWD/../lua52/src

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../core/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../core/debug/ -lcore
else:unix: LIBS += -L$$OUT_PWD/../core/ -lcore

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../editor/release/ -leditor
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../editor/debug/ -leditor
else:unix: LIBS += -L$$OUT_PWD/../editor/ -leditor

INCLUDEPATH += $$PWD/../editor
DEPENDPATH += $$PWD/../editor

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../logging/release/ -llogging
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../logging/debug/ -llogging
else:unix: LIBS += -L$$OUT_PWD/../logging/ -llogging

INCLUDEPATH += $$PWD/../logging
DEPENDPATH += $$PWD/../logging
