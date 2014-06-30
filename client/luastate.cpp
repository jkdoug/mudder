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


#include "luastate.h"

LuaState::LuaState(QObject *parent) :
    QObject(parent)
{
    m_state = luaL_newstate();
    luaL_openlibs(m_state);
}

LuaState::~LuaState()
{
    lua_close(m_state);
}

void LuaState::tracebackFunction(lua_State* L)
{
    lua_getglobal(L, LUA_DBLIBNAME);
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "traceback");

        lua_remove(L, -2);

        if (lua_isfunction(L, -1))
        {
            return;
        }
    }

    lua_pop(L, 1);
    lua_pushnil(L);
}

int LuaState::pcall(int args, int rets)
{
    return LuaState::pcall(m_state, args, rets);
}

int LuaState::pcall(lua_State *L, int args, int rets)
{
    int result = LUA_OK;
    int base = lua_gettop(L) - args;

    tracebackFunction(L);

    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        result = lua_pcall(L, args, rets, 0);
    }
    else
    {
        lua_insert(L, base);
        result = lua_pcall(L, args, rets, base);
        lua_remove(L, base);
    }

    return result;
}
