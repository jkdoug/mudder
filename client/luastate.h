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


#ifndef LUASTATE_H
#define LUASTATE_H

#include <QObject>
#include <QString>
#include "lua.hpp"

class LuaState : public QObject
{
    Q_OBJECT
public:
    explicit LuaState(QObject *parent = 0);
    ~LuaState();

    inline operator lua_State*() { return m_state; }

    int pcall(int args, int rets);
    static int pcall(lua_State *L, int args, int rets);

    static QString concatArgs(lua_State *L, const QString &delimiter = " ", const int first = 1);

private:
    static void tracebackFunction(lua_State *L);

    lua_State *m_state;
};

#endif // LUASTATE_H
