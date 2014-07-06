/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  jkdoug@gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/


#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "luastate.h"

class Console;
class Event;
class Matchable;

class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = 0);
    ~Engine();

    void initialize(Console *c);

    static void setRegistryData(lua_State *L, const QString &name, void *data);
    template <class C>
    static C * registryData(lua_State *L, const QString &name);
    template <class C>
    static C * registryObject(lua_State *L, const QString &name);

    static int panic(lua_State *L);
    void error(lua_State *L, const QString &event);

    bool execute(const QString &code, const QObject *item = 0, const QVariantList &args = QVariantList());
    bool execute(int id, const QObject *item = 0, const QVariantList &args = QVariantList());

    void processEvents(const QString &name, const QVariantList &args = QVariantList());

    void saveArguments(const QVariantList &args);
    void saveCaptures(const Matchable *item);
    void clearArguments();
    void clearCaptures();

    static int print(lua_State *L);
    static int colorTell(lua_State *L);
    static int colorNote(lua_State *L);
    static int send(lua_State *L);
    static int sendAlias(lua_State *L);
    static int sendGmcp(lua_State *L);
    static int deleteLine(lua_State *L);
    static int deleteLines(lua_State *L);
    static int simulate(lua_State *L);
    static int jsonDecode(lua_State *L);
    static int jsonEncode(lua_State *L);
    static int getVariable(lua_State *L);
    static int setVariable(lua_State *L);
    static int deleteVariable(lua_State *L);
    static int isConnected(lua_State *L);
    static int connectRemote(lua_State *L);
    static int disconnectRemote(lua_State *L);
    static int version(lua_State *L);
    static int raiseEvent(lua_State *L);
    static int registerEvent(lua_State *L);
    static int unregisterEvent(lua_State *L);

public slots:
    void enableGMCP(bool flag);
    void handleGMCP(const QString &name, const QString &args);

protected:
    int loadResource(lua_State *L, const QString &resource);

private:
    LuaState m_global;

    QString m_chunk;

    bool m_GMCP;
};

#endif // ENGINE_H
