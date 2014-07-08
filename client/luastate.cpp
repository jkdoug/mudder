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
#include "LuaBridge.h"

using namespace luabridge;


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

QString LuaState::concatArgs(lua_State *L, const QString &delimiter, const int first)
{
    int numArgs = lua_gettop(L);

    lua_getglobal(L, "tostring");

    QString output;
    for (int i = first; i <= numArgs; i++)
    {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);

        const char *s = lua_tostring(L, -1);
        if (s == 0)
        {
            QString err(tr("'%1' must return a string to be concatenated").arg("tostring"));
            luaL_error(L, qPrintable(err));
        }

        if (i > first)
        {
            output += delimiter;
        }

        output += s;

        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    return output;
}

QTextCharFormat LuaState::parseFormat(lua_State *L, int arg)
{
    QTextCharFormat fmt;

    LuaRef style(LuaRef::fromStack(L, arg));
    if (style["fg"])
    {
        QColor fg(style["fg"].cast<QString>());
        if (!fg.isValid())
        {
            luaL_argerror(L, arg, lua_pushfstring(L, qPrintable(tr("invalid color value in '%1'").arg("fg"))));
            return QTextCharFormat();
        }
        fmt.setForeground(fg);
    }

    if (style["bg"])
    {
        QColor bg(style["bg"].cast<QString>());
        if (!bg.isValid())
        {
            luaL_argerror(L, arg, lua_pushfstring(L, qPrintable(tr("invalid color value in '%1'").arg("bg"))));
            return QTextCharFormat();
        }
        fmt.setBackground(bg);
    }

    if (style["font"])
    {
        QString fontFamily(style["font"].cast<QString>());
        if (!fontFamily.isEmpty())
        {
            fmt.setFontFamily(fontFamily);
        }
    }

    if (style["size"])
    {
        fmt.setFontPointSize(style["size"].cast<int>());
    }

    if (style["bold"])
    {
        fmt.setFontWeight(style["bold"].cast<bool>()?QFont::Bold:QFont::Normal);
    }
    if (style["italic"])
    {
        fmt.setFontItalic(style["italic"].cast<bool>());
    }
    if (style["underline"])
    {
        fmt.setFontUnderline(style["underline"].cast<bool>());
    }

    return fmt;
}
