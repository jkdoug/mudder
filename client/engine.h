/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  larkin.dischai@gmail.com

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
#include "lua.hpp"

class Console;

class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = 0);
    ~Engine();

    void initialize(Console *c);

    void setRegistryData(const QString &name, void *data);
    template <class C>
    static C * registryData(const QString &name, lua_State *L);
    template <class C>
    static C * registryObject(const QString &name, lua_State *L);

    static int panic(lua_State *L);
    void error(lua_State *L, const QString &event);
    void findTraceback(lua_State *L);

    bool execute(const QString &code, const QObject * const item = 0);

    static int print(lua_State *L);
    static int send(lua_State *L);
    static int sendAlias(lua_State *L);
    static int sendGmcp(lua_State *L);

public slots:
    void enableGMCP(bool flag);
    void handleGMCP(const QString &name, const QString &args);

protected:
    int loadResource(lua_State *L, const QString &resource);

    static QString concatArgs(lua_State *L, const QString &delimiter = "", const int first = 1);

private:
    lua_State *m_global;

    QString m_chunk;

    bool m_GMCP;
};

#endif // ENGINE_H
