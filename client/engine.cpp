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


#include <QMessageBox>
#include "engine.h"
#include "LuaBridge.h"
#include "logger.h"
#include "console.h"
#include "profile.h"
#include "profileitem.h"

using namespace luabridge;

Engine::Engine(QObject *parent) :
    QObject(parent)
{
    m_global = 0;
    m_gmcpEnabled = false;
}

Engine::~Engine()
{
    if (m_global)
    {
        lua_close(m_global);
        m_global = 0;
    }
}

void Engine::initialize(Console *c)
{
    m_global = luaL_newstate();
    if (!m_global)
    {
        LOG_ERROR(tr("Failed to initialize Lua API."));

        QMessageBox mb;
        mb.setIcon(QMessageBox::Critical);
        mb.setWindowTitle(tr("Engine Trouble"));
        mb.setText(tr("Failed to initialize Lua API."));
        mb.exec();
        return;
    }

    luaL_openlibs(m_global);

    lua_atpanic(m_global, Engine::panic);

    getGlobalNamespace(m_global)
        .beginNamespace("gmcp")
            .addVariable("enabled", &m_gmcpEnabled, false)
        .endNamespace()
        .addCFunction("print", Engine::print);

    lua_settop(m_global, 0);

    setRegistryData("CONSOLE", (void *)c);
    c->printInfo("Script engine initialized.");
}

void Engine::setRegistryData(const QString &name, void *data)
{
    if (!data)
    {
        return;
    }

    lua_pushlightuserdata(m_global, data);
    lua_setfield(m_global, LUA_REGISTRYINDEX, qPrintable(name));
}

template <class C>
C * Engine::registryData(const QString &name, lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, qPrintable(name));
    C *data = static_cast<C *>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    return data;
}

template <class C>
C * Engine::registryObject(const QString &name, lua_State *L)
{
    QObject *obj = registryData<QObject>(name, L);
    return qobject_cast<C *>(obj);
}

int Engine::panic(lua_State *L)
{
    QString msg(tr("Unprotected error in call to Lua API: %1.").arg(lua_tostring(L, -1)));
    LOG_ERROR(msg);

    QMessageBox mb;
    mb.setIcon(QMessageBox::Critical);
    mb.setWindowTitle(tr("Engine Trouble"));
    mb.setText(msg);
    mb.exec();
    return 0;
}

void Engine::error(lua_State *L, const QString &event)
{
    Console *c = registryObject<Console>("CONSOLE", L);

    if (!c)
    {
        LOG_ERROR(tr("Engine::error - No active console."));
        return;
    }

    QString raisedBy(tr("No console active"));

    Profile *p = c->profile();
    if (p)
    {
//        if (!p->name().isEmpty())
//        {
//            raisedBy = tr("Profile: %1").arg(p->name());
//        }
//        else if (!p->name().isNull())
//        {
//            raisedBy = tr("Host: %1:%2").arg(p->address()).arg(p->port());
//        }
        raisedBy = tr("Profile");
    }

    QString calledBy(tr("Immediate execution"));
    ProfileItem *item = registryObject<ProfileItem>("CALLER", L);
    if (item)
    {
        calledBy = tr("Reason: processing %1 \"%2\"").arg(item->tagName()).arg(item->name());
    }

    QString errDescription(lua_tostring(L, -1));

    lua_settop(L, 0);

    QStringList printLines;
    printLines << event << raisedBy << calledBy << errDescription;

    bool okLine = false;
    int errLine = errDescription.section(":", 1, 1).toInt(&okLine);
    if (okLine && !m_chunk.isEmpty())
    {
        printLines.clear();
        QStringList errLines(m_chunk.split("\n"));
        if (errLine <= errLines.size())
        {
            printLines << tr("Error context in script:");

            int start = qMax(errLine - 4, 1);
            int end = qMin(errLine + 4, errLines.size());

            for (int line = start; line <= end; line++)
            {
                printLines << tr("%1%2: %3").arg(line, 4).arg(line == errLine?"*":" ").arg(errLines.at(line - 1));
            }

        }
    }

    c->printError(printLines.join('\n'));
}

inline void Engine::findTraceback(lua_State *L)
{
    lua_getglobal(L, "debug");

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

bool Engine::execute(const QString &code, const QObject * const item)
{
    if (code.isEmpty())
    {
        return true;
    }

    if (!m_global)
    {
        LOG_ERROR(tr("No Lua execution unit found."));
        return false;
    }

    // Start with an empty stack
    lua_settop(m_global, 0);

    // Give access to the caller object
    setRegistryData("CALLER", (void *)item);

    // Store code chunk for error displays
    m_chunk = code;

    // Load up the code
    int err = luaL_loadstring(m_global, qPrintable(code));
    if (err != 0)
    {
        error(m_global, tr("Compile error"));
        return false;
    }

    // Prepare for debugging the stack
    int base = lua_gettop(m_global);
    findTraceback(m_global);

    // Execute the code and look for an error value
    if (lua_isnil(m_global, -1))
    {
        lua_pop(m_global, 1);
        err = lua_pcall(m_global, 0, LUA_MULTRET, 0);
    }
    else
    {
        lua_insert(m_global, base);
        err = lua_pcall(m_global, 0, LUA_MULTRET, base);
        lua_remove(m_global, base);
    }

    // Something didn't work, print it out
    if (err != 0)
    {
        error(m_global, tr("Run-time error"));
        return false;
    }

    lua_settop(m_global, 0);
    m_chunk.clear();

    return err == 0;
}

int Engine::print(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);
    c->printInfo(concatArgs(L, " "));

    return 0;
}

QString Engine::concatArgs(lua_State *L, const QString &delimiter, const int first)
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
            luaL_error(L, qPrintable(tr("'%1' must return a string to be concatenated").arg("tostring")));
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
