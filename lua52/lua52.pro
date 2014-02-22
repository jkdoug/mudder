#-------------------------------------------------
#
# Project created by QtCreator 2013-05-06T15:11:43
#
#-------------------------------------------------

QT -= core gui

CONFIG += staticlib

TARGET = lua52
TEMPLATE = lib

DEFINES += _CRT_SECURE_NO_WARNINGS=1
win32: DEFINES += LUA_BUILD_AS_DLL

SOURCES += \
    src/lzio.c \
    src/lvm.c \
    src/lundump.c \
    src/ltm.c \
    src/ltablib.c \
    src/ltable.c \
    src/lstrlib.c \
    src/lstring.c \
    src/lstate.c \
    src/lparser.c \
    src/loslib.c \
    src/lopcodes.c \
    src/lobject.c \
    src/loadlib.c \
    src/lmem.c \
    src/lmathlib.c \
    src/llex.c \
    src/liolib.c \
    src/linit.c \
    src/lgc.c \
    src/lfunc.c \
    src/ldump.c \
    src/ldo.c \
    src/ldebug.c \
    src/ldblib.c \
    src/lctype.c \
    src/lcorolib.c \
    src/lcode.c \
    src/lbitlib.c \
    src/lbaselib.c \
    src/lauxlib.c \
    src/lapi.c

HEADERS +=\
    src/lzio.h \
    src/lvm.h \
    src/lundump.h \
    src/lualib.h \
    src/luaconf.h \
    src/lua.hpp \
    src/lua.h \
    src/ltm.h \
    src/ltable.h \
    src/lstring.h \
    src/lstate.h \
    src/lparser.h \
    src/lopcodes.h \
    src/lobject.h \
    src/lmem.h \
    src/llimits.h \
    src/llex.h \
    src/lgc.h \
    src/lfunc.h \
    src/ldo.h \
    src/ldebug.h \
    src/lctype.h \
    src/lcode.h \
    src/lauxlib.h \
    src/lapi.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
