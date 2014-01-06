/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "engine.h"
#include "alias.h"
#include "console.h"
#include "group.h"
#include "luawrapper.h"
#include "matchable.h"
#include "profile.h"
#include "xmlexception.h"
#include <QDebug>
#include <QDomDocument>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>
#include <QStringList>
#include <QUrl>
#include <QVariantList>

inline bool lua_isarray(lua_State *L, int arg)
{
    if (!lua_istable(L, arg))
    {
        return false;
    }

    lua_pushvalue(L, arg);
    lua_pushnil(L);

    while (lua_next(L, -2))
    {
        lua_pushvalue(L, -2);

        if (!lua_isnumber(L, -1))
        {
            return false;
        }

        lua_pop(L, 2);
    }

    lua_pop(L, 1);

    return true;
}


Engine::Engine(QObject *parent) :
    QObject(parent)
{
    m_global = 0;
}

Engine::~Engine()
{
    if (m_global)
    {
        lua_close(m_global);
        m_global = 0;
    }
}

void Engine::initialize(Console *c, const QString &script)
{
    Q_ASSERT(c != 0);

    m_global = luaL_newstate();
    if (!m_global)
    {
        QMessageBox mb;
        mb.setIcon(QMessageBox::Critical);
        mb.setWindowTitle(tr("Engine Trouble"));
        mb.setText(tr("Failed to initialize Lua API."));
        mb.exec();
        return;
    }

    lua_atpanic(m_global, Engine::panic);

    luaL_openlibs(m_global);

    lua_pushlightuserdata(m_global, (void *)c);
    lua_setfield(m_global, LUA_REGISTRYINDEX, "CONSOLE");

    lua_pushlightuserdata(m_global, (void *)(new EventMap));
    lua_setfield(m_global, LUA_REGISTRYINDEX, "HANDLERS");

    registerEverything(m_global);

    lua_register(m_global, "print", Engine::print);

    lua_register(m_global, "ColorNote", Engine::colorNote);
    lua_register(m_global, "ColorTell", Engine::colorTell);
    lua_register(m_global, "ColourNote", Engine::colorNote);
    lua_register(m_global, "ColourTell", Engine::colorTell);
    lua_register(m_global, "Note", Engine::note);
    lua_register(m_global, "Tell", Engine::tell);
    lua_register(m_global, "Hyperlink", Engine::hyperlink);

    lua_register(m_global, "Send", Engine::send);
    lua_register(m_global, "Execute", Engine::sendAlias);
    lua_register(m_global, "SendGmcp", Engine::sendGmcp);
    lua_register(m_global, "Simulate", Engine::simulate);

    lua_register(m_global, "OpenLog", Engine::openLog);
    lua_register(m_global, "LogNote", Engine::logNote);
    lua_register(m_global, "CloseLog", Engine::closeLog);
    lua_register(m_global, "IsLogging", Engine::isLogging);
    lua_register(m_global, "PauseLog", Engine::pauseLog);

    lua_register(m_global, "SetVariable", Engine::setVariable);
    lua_register(m_global, "GetVariable", Engine::getVariable);
    lua_register(m_global, "DeleteVariable", Engine::deleteVariable);

    lua_register(m_global, "JsonDecode", Engine::jsonDecode);
    lua_register(m_global, "JsonEncode", Engine::jsonEncode);

    lua_register(m_global, "IsConnected", Engine::isConnected);
    lua_register(m_global, "Connect", Engine::connect);
    lua_register(m_global, "Disconnect", Engine::disconnect);

    lua_register(m_global, "PlaySound", Engine::playSound);

    lua_settop(m_global, 0);

    Q_ASSERT(loadResource(m_global, ":/lua/io_exists") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/string_commas") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/string_explode") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/string_join") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/string_split") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/string_title") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/string_trim") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/string_wrap") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/table_contains") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/table_ordered") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/table_print") == LUA_OK);
    Q_ASSERT(loadResource(m_global, ":/lua/table_size") == LUA_OK);

    if (!script.isEmpty())
    {
        execute(script);
    }
}

static const luaL_Reg Profile_table[] = {
    { "prop", Engine::profileProperty },
    { "activeGroup", Engine::profileActiveGroup },
    { "importXml", Engine::profileImportXml },
    { 0, 0 },
};

static const luaL_Reg Accelerator_table[] = {
    { "get", Engine::acceleratorGet },
    { 0, 0 },
};

static const luaL_Reg Accelerator_metatable[] = {
    { "prop", Engine::acceleratorProperty },
    { "delete", Engine::acceleratorDelete },
    { "enable", Engine::acceleratorEnable },
    { "__call", Engine::acceleratorFire },
    { "__tostring", Engine::acceleratorString },
    { "xml", Engine::acceleratorXml },
    { 0, 0 },
};

static const luaL_Reg Alias_table[] = {
    { "get", Engine::aliasGet },
    { 0, 0 },
};

static const luaL_Reg Alias_metatable[] = {
    { "prop", Engine::aliasProperty },
    { "delete", Engine::aliasDelete },
    { "enable", Engine::aliasEnable },
    { "__call", Engine::aliasFire },
    { "__tostring", Engine::aliasString },
    { "xml", Engine::aliasXml },
    { 0, 0 },
};

static const luaL_Reg Event_table[] = {
    { "list", Engine::eventList },
    { "unregister", Engine::eventUnregister },
    { "raise", Engine::eventRaise },
    { 0, 0 },
};

static const luaL_Reg Event_metatable[] = {
    { "prop", Engine::eventProperty },
    { "__tostring", Engine::eventString },
    { "unregister", Engine::eventUnregister },
    { "xml", Engine::eventXml },
    { 0, 0 },
};

static const luaL_Reg Group_table[] = {
    { "get", Engine::groupGet },
    { 0, 0 },
};

static const luaL_Reg Group_metatable[] = {
    { "prop", Engine::groupProperty },
    { "delete", Engine::groupDelete },
    { "enable", Engine::groupEnable },
    { "xml", Engine::groupXml },
    { "accelerators", Engine::groupAccelerators },
    { "aliases", Engine::groupAliases },
    { "groups", Engine::groupGroups },
    { "timers", Engine::groupTimers },
    { "triggers", Engine::groupTriggers },
    { "variables", Engine::groupVariables },
    { "__tostring", Engine::groupString },
    { 0, 0 },
};

static const luaL_Reg Timer_table[] = {
    { "get", Engine::timerGet },
    { 0, 0 },
};

static const luaL_Reg Timer_metatable[] = {
    { "prop", Engine::timerProperty },
    { "delete", Engine::timerDelete },
    { "enable", Engine::timerEnable },
    { "__call", Engine::timerFire },
    { "__tostring", Engine::timerString },
    { "xml", Engine::timerXml },
    { 0, 0 },
};

static const luaL_Reg Trigger_table[] = {
    { "get", Engine::triggerGet },
    { "test", Engine::triggerTest },
    { 0, 0 },
};

static const luaL_Reg Trigger_metatable[] = {
    { "prop", Engine::triggerProperty },
    { "delete", Engine::triggerDelete },
    { "enable", Engine::triggerEnable },
    { "__call", Engine::triggerFire },
    { "__tostring", Engine::triggerString },
    { "xml", Engine::triggerXml },
    { 0, 0 },
};

static const luaL_Reg Variable_table[] = {
    { "get", Engine::variableGet },
    { "delete", Engine::deleteVariable },
    { 0, 0 },
};

static const luaL_Reg Variable_metatable[] = {
    { "prop", Engine::variableProperty },
    { "delete", Engine::variableDelete },
    { "__tostring", Engine::variableString },
    { "xml", Engine::variableXml },
    { 0, 0 },
};

void Engine::registerEverything(lua_State * L)
{
    luaW_register<Profile>(L, "Profile", Profile_table, 0, 0, 0);

    luaW_register<Accelerator>(L, "Accelerator", Accelerator_table, Accelerator_metatable, Engine::acceleratorNew, 0);
    luaW_register<Alias>(L, "Alias", Alias_table, Alias_metatable, Engine::aliasNew, 0);
    luaW_register<Event>(L, "Event", Event_table, Event_metatable, Engine::eventNew, 0);
    luaW_register<Group>(L, "Group", Group_table, Group_metatable, Engine::groupNew, 0);
    luaW_register<Timer>(L, "Timer", Timer_table, Timer_metatable, Engine::timerNew, 0);
    luaW_register<Trigger>(L, "Trigger", Trigger_table, Trigger_metatable, Engine::triggerNew, 0);
    luaW_register<Variable>(L, "Variable", Variable_table, Variable_metatable, Engine::variableNew, 0);
}

int Engine::loadResource(lua_State *L, const QString &resource)
{
    QFile res(resource);
    res.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&res);
    QString code(ts.readAll());

    int ret = luaL_dostring(L, code.toLatin1().data());
    if (ret != LUA_OK)
    {
        luaL_error(L, tr("failed to load resource '%1': %2").arg(resource).arg(lua_tostring(L, -1)).toLatin1().data());
    }

    return ret;
}

bool Engine::compile(const QString &code, const QString &what, QString *err)
{
    if (code.isEmpty())
    {
        return true;
    }

    lua_State *L = luaL_newstate();

    if (!L)
    {
        if (err)
        {
            *err = tr("No Lua execution unit found");
        }
        return false;
    }

    if (luaL_loadbuffer(L, code.toLatin1().data(), code.length(), what.toLatin1().data()))
    {
        if (err)
        {
            *err = lua_tostring(L, -1);
        }
        lua_close(L);
        return false;
    }

    lua_close(L);
    return true;
}

inline bool Engine::execute(const QString &code, QObject *item)
{
    if (code.isEmpty())
    {
        return true;
    }

    if (!m_global)
    {
        qCritical() << "LUA CRITICAL ERROR: no Lua execution unit found.";
        return false;
    }

    // Start with an empty stack
    lua_settop(m_global, 0);

    // Give access to the caller object
    lua_pushlightuserdata(m_global, (void *)item);
    lua_setfield(m_global, LUA_REGISTRYINDEX, "CALLER");

    // Store code chunk for error displays
    m_chunk = code;

    // Load up the code
    int err = luaL_loadstring(m_global, code.toLatin1().data());
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

bool Engine::execute(int id, QObject *item)
{
    if (id < 1)
    {
        return false;
    }

    if (!m_global)
    {
        qCritical() << "LUA CRITICAL ERROR: no Lua execution unit found.";
        return false;
    }

    // Start with an empty stack
    lua_settop(m_global, 0);

    // Give access to the caller object
    lua_pushlightuserdata(m_global, (void *)item);
    lua_setfield(m_global, LUA_REGISTRYINDEX, "CALLER");

    lua_rawgeti(m_global, LUA_REGISTRYINDEX, id);

    int err = lua_pcall(m_global, 0, 0, 0);

    // Something didn't work, print it out
    if (err != 0)
    {
        error(m_global, tr("Run-time error"));
        return false;
    }

    lua_settop(m_global, 0);

    return err == 0;
}

inline int Engine::execute(lua_State *L, const QString &code, QObject *item)
{
    if (code.isEmpty())
    {
        return 0;
    }

    lua_settop(L, 0);

    lua_pushlightuserdata(L, (void *)item);
    lua_setfield(L, LUA_REGISTRYINDEX, "CALLER");

    // Load up the code
    int err = luaL_loadstring(L, code.toLatin1().data());
    if (err != 0)
    {
        return err;
    }

    // Execute the code and look for an error value
    err = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (err != 0)
    {
        return err;
    }

    lua_settop(L, 0);

    return 0;
}

inline int Engine::callEventHandler(lua_State *L, Event *event, const QVariantList &args)
{
    Q_ASSERT(event != 0);

    lua_rawgeti(L, LUA_REGISTRYINDEX, event->reference());

    foreach (QVariant arg, args)
    {
        pushVariant(L, arg);
    }

    int result = lua_pcall(L, args.size(), 0, 0);
    if (result == 0)
    {
        lua_settop(L, 0);
    }

    return result;
}

void Engine::processEvent(const QString &name, const QVariantList &args)
{
    if (!m_global)
    {
        return;
    }

    lua_settop(m_global, 0);

    EventMap *h = handlers(m_global);
    if (!h->contains(name))
    {
        return;
    }

    QList<Event *> events = h->value(name);
    foreach (Event *event, events)
    {
        callEventHandler(m_global, event, args);
    }
}

void Engine::saveCaptures(const Matchable * const item)
{
    if (!m_global)
    {
        qCritical() << "LUA CRITICAL ERROR: no Lua execution unit found.";
        return;
    }

    saveCaptures(m_global, item);
}

void Engine::saveCaptures(lua_State *L, const Matchable * const item)
{
    if (!item || !item->lastMatch())
    {
        return;
    }

    lua_getglobal(L, "matches");
    if (!lua_istable(L, -1))
    {
        lua_newtable(L);
    }

    for (int capture = 0; capture <= item->lastMatch()->lastCapturedIndex(); capture++)
    {
        lua_pushnumber(L, capture);
        lua_pushstring(L, item->lastMatch()->captured(capture).toLatin1().data());
        lua_settable(L, -3);
    }

    foreach (QString name, item->regex().namedCaptureGroups())
    {
        if (!name.isEmpty())
        {
            lua_pushstring(L, name.toLatin1().data());
            lua_pushstring(L, item->lastMatch()->captured(name).toLatin1().data());
            lua_settable(L, -3);
        }
    }

    lua_setglobal(L, "matches");
}

inline int Engine::typeError(lua_State *L, int narg, const char *tname)
{
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

inline void Engine::tagError(lua_State *L, int narg, int tag)
{
    typeError(L, narg, lua_typename(L, tag));
}

inline QString Engine::checkString(lua_State *L, int arg)
{
    return luaL_checklstring(L, arg, 0);
}

inline double Engine::checkNumber(lua_State *L, int arg)
{
    return luaL_checknumber(L, arg);
}

inline int Engine::checkInteger(lua_State *L, int arg)
{
    return luaL_checkinteger(L, arg);
}

inline QVariant Engine::checkVariant(lua_State *L, int arg, int type)
{
    QVariant var;
    if (lua_isnumber(L, arg))
    {
        if (type == QVariant::Int)
        {
            var = QVariant(checkInteger(L, arg));
        }
        else
        {
            var = QVariant(checkNumber(L, arg));
        }
    }
    else if (lua_isboolean(L, arg))
    {
        var = QVariant(lua_toboolean(L, arg) != 0);
    }
    else if (lua_isarray(L, arg))
    {
        QVariantList list;

        lua_pushvalue(L, arg);
        lua_pushnil(L);

        while (lua_next(L, -2))
        {
            lua_pushvalue(L, -2);

            list.insert(lua_tonumber(L, -1), checkVariant(L, -2));

            lua_pop(L, 2);
        }

        lua_pop(L, 1);

        var = list;
    }
    else if (lua_istable(L, arg))
    {
        QVariantMap map;

        lua_pushvalue(L, arg);
        lua_pushnil(L);

        while (lua_next(L, -2))
        {
            lua_pushvalue(L, -2);

            map.insert(lua_tostring(L, -1), checkVariant(L, -2));

            lua_pop(L, 2);
        }

        lua_pop(L, 1);

        var = map;
    }
    else
    {
        var = QVariant(checkString(L, arg));
    }

    if (type != QVariant::Invalid && !var.isValid())
    {
        return luaL_argerror(L, arg, lua_pushfstring(L, tr("conversion to %1 failed").arg(QVariant::typeToName(type)).toLatin1().data()));
    }

    return var;
}


inline QString Engine::optionalString(lua_State *L, int arg, const QString &def)
{
    return luaL_optlstring(L, arg, def.toLatin1().data(), 0);
}

inline lua_Number Engine::optionalNumber(lua_State *L, int arg, lua_Number def)
{
    return luaL_optnumber(L, arg, def);
}

inline bool Engine::optionalBoolean(lua_State *L, int arg, bool def)
{
    // Argument not present, take default
    if (lua_gettop(L) < arg)
    {
        return def;
    }

    // Nil will default
    if (lua_isnil(L, arg))
    {
        return def;
    }

    // Try to convert boolean
    if (lua_isboolean(L, arg))
    {
        return lua_toboolean(L, arg);
    }

    return checkNumber(L, arg) != 0;
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
            luaL_error(L, tr("'%1' must return a string to be concatenated").arg("tostring").toLatin1().data());
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

void Engine::pushVariant(lua_State *L, const QVariant &value)
{
    switch (value.type())
    {
    case QMetaType::Bool:
        lua_pushboolean(L, value.toBool());
        break;

    case QMetaType::Double:
        lua_pushnumber(L, value.toDouble());
        break;

    case QMetaType::Int:
        lua_pushnumber(L, value.toInt());
        break;

    case QMetaType::QRegularExpression:
    {
        QRegularExpression regex(value.value<QRegularExpression>());
        lua_newtable(L);
        lua_pushliteral(L, "pattern");
        lua_pushstring(L, regex.pattern().toLatin1().data());
        lua_settable(L, -3);
        lua_pushliteral(L, "options");
        lua_pushnumber(L, regex.patternOptions());
        lua_settable(L, -3);
        lua_pushliteral(L, "valid");
        lua_pushboolean(L, regex.isValid());
        lua_settable(L, -3);
        if (!regex.errorString().isEmpty())
        {
            lua_pushliteral(L, "error");
            lua_pushstring(L, regex.errorString().toLatin1().data());
            lua_settable(L, -3);
            lua_pushliteral(L, "errorOffset");
            lua_pushnumber(L, regex.patternErrorOffset());
            lua_settable(L, -3);
        }
    }
        break;

    case QMetaType::QPoint:
    {
        QPoint point(value.value<QPoint>());
        lua_newtable(L);
        lua_pushliteral(L, "x");
        lua_pushnumber(L, point.x());
        lua_settable(L, -3);
        lua_pushliteral(L, "y");
        lua_pushnumber(L, point.y());
        lua_settable(L, -3);
    }
        break;

    case QMetaType::QFont:
    {
        QFont font(value.value<QFont>());
        lua_newtable(L);
        lua_pushliteral(L, "family");
        lua_pushstring(L, font.family().toLatin1().data());
        lua_settable(L, -3);
        lua_pushliteral(L, "size");
        lua_pushnumber(L, font.pointSize());
        lua_settable(L, -3);
        lua_pushliteral(L, "antialias");
        lua_pushboolean(L, font.styleStrategy() & QFont::PreferAntialias);
        lua_settable(L, -3);
    }
        break;

    case QMetaType::QJsonDocument:
    {
        QJsonDocument json(value.toJsonDocument());

        if (json.isNull())
        {
            lua_pushnil(L);
            return;
        }

        lua_newtable(L);

        if (json.isEmpty())
        {
            return;
        }

        if (json.isArray())
        {
            pushVariant(L, json.array());
        }
        else if (json.isObject())
        {
            pushVariant(L, json.object());
        }
    }
        break;

    case QMetaType::QJsonArray:
    {
        Q_ASSERT(lua_istable(L, -1));

        QJsonArray json(value.toJsonArray());

        int n = 0;
        foreach (QJsonValue element, json)
        {
            lua_pushnumber(L, ++n);
            pushVariant(L, element);
            lua_settable(L, -3);
        }
    }
        break;

    case QMetaType::QJsonObject:
    {
        Q_ASSERT(lua_istable(L, -1));

        QJsonObject json(value.toJsonObject());

        QStringList keys(json.keys());
        foreach (QString key, keys)
        {
            lua_pushstring(L, key.toLatin1().data());
            pushVariant(L, json.value(key));
            lua_settable(L, -3);
        }
    }
        break;

    case QMetaType::QJsonValue:
    {
        QJsonValue json(value.toJsonValue());
        if (json.isArray())
        {
            lua_newtable(L);
            pushVariant(L, json.toArray());
        }
        else if (json.isObject())
        {
            lua_newtable(L);
            pushVariant(L, json.toObject());
        }
        else
        {
            pushVariant(L, json.toVariant());
        }
    }
        break;

    default:
        lua_pushstring(L, value.toString().toLatin1().data());
        break;
    }
}

void Engine::pushXml(lua_State *L, ProfileItem *item, bool format)
{
    if (item == 0)
    {
        lua_pushnil(L);
        return;
    }

    QString output;
    QXmlStreamWriter xml(&output);
    if (format)
    {
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(2);
    }

    try
    {
        item->toXml(xml);
        lua_pushstring(L, output.toLatin1().data());
    }
    catch (XmlException *xe)
    {
        QString err;
        for (int w = 0; w < xe->warnings().size(); w++)
        {
            err += QString(" %1;").arg(xe->warnings().at(w));
        }
        luaL_error(L, err.toLatin1().data());
    }
}

void Engine::makeTableItem(lua_State *L, const QString &name, const QVariant &value)
{
    if (!value.isValid() || value.isNull())
    {
        return;
    }

    switch (value.type())
    {
    case QVariant::Bool:
        lua_pushstring(L, name.toLatin1().data());
        lua_pushboolean(L, value.toBool());
        lua_rawset(L, -3);
        break;

    case QVariant::Double:
        lua_pushstring(L, name.toLatin1().data());
        lua_pushnumber(L, value.toDouble());
        lua_rawset(L, -3);
        break;

    case QVariant::Int:
        lua_pushstring(L, name.toLatin1().data());
        lua_pushnumber(L, value.toInt());
        lua_rawset(L, -3);
        break;

    case QVariant::DateTime:
        lua_pushstring(L, name.toLatin1().data());
        lua_pushstring(L, value.toDateTime().toString().toLatin1().data());
        lua_rawset(L, -3);
        break;

    case QVariant::RegularExpression:
    {
        QRegularExpression regex(value.value<QRegularExpression>());
        lua_newtable(L);
        lua_pushliteral(L, "pattern");
        lua_pushstring(L, regex.pattern().toLatin1().data());
        lua_settable(L, -3);
        lua_pushliteral(L, "options");
        lua_pushnumber(L, regex.patternOptions());
        lua_settable(L, -3);
        lua_pushliteral(L, "valid");
        lua_pushboolean(L, regex.isValid());
        lua_settable(L, -3);
        if (!regex.errorString().isEmpty())
        {
            lua_pushliteral(L, "error");
            lua_pushstring(L, regex.errorString().toLatin1().data());
            lua_settable(L, -3);
            lua_pushliteral(L, "errorOffset");
            lua_pushnumber(L, regex.patternErrorOffset());
            lua_settable(L, -3);
        }

        lua_pushstring(L, name.toLatin1().data());
        lua_insert(L, -2);
        lua_settable(L, -3);
    }
        break;

    case QMetaType::QPoint:
    {
        QPoint point(value.value<QPoint>());
        lua_newtable(L);
        lua_pushliteral(L, "x");
        lua_pushnumber(L, point.x());
        lua_settable(L, -3);
        lua_pushliteral(L, "y");
        lua_pushnumber(L, point.y());
        lua_settable(L, -3);

        lua_pushstring(L, name.toLatin1().data());
        lua_insert(L, -2);
        lua_settable(L, -3);
    }
        break;

    case QVariant::Font:
    {
        QFont font(value.value<QFont>());
        lua_newtable(L);
        lua_pushliteral(L, "family");
        lua_pushstring(L, font.family().toLatin1().data());
        lua_settable(L, -3);
        lua_pushliteral(L, "size");
        lua_pushnumber(L, font.pointSize());
        lua_settable(L, -3);
        lua_pushliteral(L, "antialias");
        lua_pushboolean(L, font.styleStrategy() & QFont::PreferAntialias);
        lua_settable(L, -3);

        lua_pushstring(L, name.toLatin1().data());
        lua_insert(L, -2);
        lua_settable(L, -3);
    }
        break;

    default:
        lua_pushstring(L, name.toLatin1().data());
        lua_pushstring(L, value.toString().toLatin1().data());
        lua_rawset(L, -3);
        break;
    }
}

int Engine::panic(lua_State *L)
{
    QMessageBox mb;
    mb.setIcon(QMessageBox::Critical);
    mb.setWindowTitle(tr("Engine Trouble"));
    mb.setText(tr("Unprotected error in call to Lua API: %1.").arg(lua_tostring(L, -1)));
    mb.exec();
    return 0;
}

void Engine::error(lua_State *L, const QString &event)
{
    Console *c = console(L);

    if (!c)
    {
        qCritical() << "Engine::error - no valid console";
        return;
    }

    QString raisedBy(tr("No console active"));

    Profile *p = c->profile();
    if (p)
    {
        if (!p->name().isEmpty())
        {
            raisedBy = tr("Profile: %1").arg(p->name());
        }
        else if (!p->name().isNull())
        {
            raisedBy = tr("Host: %1:%2").arg(p->address()).arg(p->port());
        }
    }

    QString calledBy(tr("Immediate execution"));
    ProfileItem *item = qobject_cast<ProfileItem *>(caller(L));
    if (item)
    {
        calledBy = tr("Reason: processing %1 \"%2\"").arg(item->metaObject()->className()).arg(item->name());
    }

    QString errDescription(lua_tostring(L, -1));

    lua_settop(L, 0);

    c->systemErr(event);
    c->systemErr(raisedBy);
    c->systemErr(calledBy);
    c->systemErr(errDescription);

    bool okLine = false;
    int errLine = errDescription.section(":", 1, 1).toInt(&okLine);
    if (okLine && !m_chunk.isEmpty())
    {
        QStringList errLines(m_chunk.split("\n"));
        if (errLine <= errLines.size())
        {
            c->systemErr(tr("Error context in script:"));

            int start = qMax(errLine - 4, 1);
            int end = qMin(errLine + 4, errLines.size());

            for (int line = start; line <= end; line++)
            {
                c->systemErr(tr("%1%2: %3").arg(line, 4).arg(line == errLine?"*":" ").arg(errLines.at(line - 1)));
            }
        }
    }
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

inline Console * Engine::console(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "CONSOLE");
    Console *c = static_cast<Console *>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    Q_ASSERT(c != 0);

    return c;
}

inline Profile * Engine::profile(lua_State *L)
{
    return console(L)->profile();
}

inline QObject * Engine::caller(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "CALLER");
    QObject *e = static_cast<QObject *>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    return e;
}

inline EventMap *Engine::handlers(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "HANDLERS");
    EventMap *h = static_cast<EventMap *>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    Q_ASSERT(h != 0);

    return h;
}

int Engine::getProperty(lua_State *L, QObject *obj, int nameArg)
{
    if (lua_isnone(L, nameArg))
    {
        lua_newtable(L);

        const QMetaObject *meta = obj->metaObject();
        int count = meta->propertyCount();
        for (int i = 0; i < count; i++)
        {
            QMetaProperty metaProperty(meta->property(i));

            makeTableItem(L, metaProperty.name(), metaProperty.read(obj));
        }

        return 1;
    }

    QString prop(checkString(L, nameArg));
    QVariant val(obj->property(prop.toLatin1().data()));
    if (!val.isValid())
    {
        return luaL_error(L, tr("invalid property: %1").arg(prop).toLatin1().data());
    }

    pushVariant(L, val);
    return 1;
}

int Engine::setProperty(lua_State *L, QObject *obj, int nameArg, int valueArg)
{
    QString prop(checkString(L, nameArg));
    QVariant val(obj->property(prop.toLatin1().data()));
    if (!val.isValid())
    {
        return luaL_error(L, tr("invalid property: %1").arg(prop).toLatin1().data());
    }

    QVariant newVal(checkVariant(L, valueArg, val.type()));
    if (!obj->setProperty(prop.toLatin1().data(), newVal))
    {
        return luaL_error(L, tr("failed to set property '%1' to '%2'").arg(prop).arg(newVal.toString()).toLatin1().data());
    }

    newVal = obj->property(prop.toLatin1().data());

    if (val != newVal)
    {
        profile(L)->setDirty(true);
        lua_pushboolean(L, true);
    }
    else
    {
        lua_pushboolean(L, false);
    }

    return 1;
}


int Engine::profileActiveGroup(lua_State *L)
{
    Profile *p = profile(L);

    if (lua_isnone(L, 1))
    {
        luaW_push<Group>(L, p->activeGroup());
        return 1;
    }

    Group *group = 0;
    if (lua_isstring(L, 1))
    {
        group = p->findGroup(checkString(L, 1));
    }
    else
    {
        group = luaW_check<Group>(L, 1);
    }

    if (group)
    {
        p->setActiveGroup(group);
    }

    lua_pushboolean(L, group != 0);
    return 1;
}

int Engine::profileProperty(lua_State *L)
{
    if (lua_isnone(L, 2))
    {
        return getProperty(L, profile(L), 1);
    }

    return setProperty(L, profile(L), 1, 2);
}

int Engine::profileImportXml(lua_State *L)
{
    Profile *p = profile(L);

    try
    {
        QXmlStreamReader xml(tr("<%1>%2</%1>").arg("importXml").arg(checkString(L, 1)));
        p->appendXml(xml);
    }
    catch (XmlException *xe)
    {
        QString err;
        for (int w = 0; w < xe->warnings().size(); w++)
        {
            err += QString(" %1;").arg(xe->warnings().at(w));
        }
        return luaL_error(L, tr("invalid XML: %1").arg(err).toLatin1().data());
    }

    return 0;
}


int Engine::itemEnable(lua_State *L, ProfileItem *item)
{
    item->enable(optionalBoolean(L, 2, true));
    return 0;
}

int Engine::itemProperty(lua_State *L, ProfileItem *item)
{
    if (lua_isnone(L, 3))
    {
        return getProperty(L, item, 2);
    }

    return setProperty(L, item, 2, 3);
}

int Engine::itemXml(lua_State *L, ProfileItem *item, bool format)
{
    pushXml(L, item, format);
    return 1;
}


Accelerator * Engine::acceleratorNew(lua_State *L)
{
    Profile *p = profile(L);

    QString name(checkString(L, 1));
    QString key(checkString(L, 2));
    QString contents(checkString(L, 3));

    if (name.isEmpty())
    {
        luaL_error(L, tr("cannot create an unnamed accelerator").toLatin1().data());
        return 0;
    }

    if (key.isEmpty())
    {
        luaL_error(L, tr("cannot create accelerator with no key sequence").toLatin1().data());
        return 0;
    }

    QKeySequence keySequence(key, QKeySequence::PortableText);
    if (keySequence.isEmpty())
    {
        luaL_error(L, tr("cannot create accelerator with invalid key sequence: %1").arg(key).toLatin1().data());
        return 0;
    }

    if (contents.isEmpty())
    {
        luaL_error(L, tr("cannot create accelerator without script").toLatin1().data());
        return 0;
    }

    Accelerator *accelerator = p->findAccelerator(name, p->rootGroup());
    if (accelerator)
    {
        luaL_error(L, tr("cannot create duplicate accelerator with name '%1'").arg(name).toLatin1().data());
        return 0;
    }

    accelerator = new Accelerator;

    accelerator->setName(name);
    accelerator->setKey(keySequence);
    accelerator->setContents(contents);

    p->addAccelerator(accelerator);

    return accelerator;
}

int Engine::acceleratorGet(lua_State *L)
{
    Profile *p = profile(L);

    if (lua_isnone(L, 1))
    {
        lua_newtable(L);

        int i = 0;
        QList<Accelerator *> accelerators = p->rootGroup()->sortedAccelerators();
        foreach (Accelerator *accelerator, accelerators)
        {
            lua_pushnumber(L, ++i);
            luaW_push<Accelerator>(L, accelerator);
        }

        return 1;
    }

    luaW_push<Accelerator>(L, p->findAccelerator(checkString(L, 1)));
    return 1;
}

int Engine::acceleratorDelete(lua_State *L)
{
    Profile *p = profile(L);

    Accelerator *accelerator = luaW_check<Accelerator>(L, 1);

    bool result = p->deleteAccelerator(accelerator);

    lua_pushnil(L);
    lua_setmetatable(L, -2);

    lua_pushboolean(L, result);
    return 1;
}

int Engine::acceleratorFire(lua_State *L)
{
    Profile *p = profile(L);

    Accelerator *accelerator = luaW_check<Accelerator>(L, 1);
    Q_ASSERT(accelerator != 0);

    Group *previousGroup = p->activeGroup();
    Q_ASSERT(previousGroup != 0);
    p->setActiveGroup(accelerator->group());

    int err = execute(L, accelerator->contents(), accelerator);
    if (err != 0)
    {
        lua_pushboolean(L, false);
        lua_pushstring(L, tr("Lua error %1: %2").arg(err).arg(lua_tostring(L, -2)).toLatin1().data());
        return 2;
    }

    p->setActiveGroup(previousGroup);

    lua_pushboolean(L, true);
    return 1;
}

int Engine::acceleratorEnable(lua_State *L)
{
    return itemEnable(L, luaW_check<Accelerator>(L, 1));
}

int Engine::acceleratorProperty(lua_State *L)
{
    return itemProperty(L, luaW_check<Accelerator>(L, 1));
}

int Engine::acceleratorString(lua_State *L)
{
    return itemXml(L, luaW_check<Accelerator>(L, 1), false);
}

int Engine::acceleratorXml(lua_State *L)
{
    return itemXml(L, luaW_check<Accelerator>(L, 1), optionalBoolean(L, 2, true));
}

Alias * Engine::aliasNew(lua_State *L)
{
    Profile *p = profile(L);

    QString name(checkString(L, 1));
    QString pattern(checkString(L, 2));
    QString contents(checkString(L, 3));

    if (name.isEmpty())
    {
        luaL_error(L, tr("cannot create an unnamed alias").toLatin1().data());
        return 0;
    }

    if (pattern.isEmpty())
    {
        luaL_error(L, tr("cannot create alias with empty pattern").toLatin1().data());
        return 0;
    }

    if (contents.isEmpty())
    {
        luaL_error(L, tr("cannot create alias without script").toLatin1().data());
        return 0;
    }

    QRegularExpression regex(pattern);
    if (!regex.isValid())
    {
        luaL_error(L, tr("invalid regular expression: %1 (column %2)").arg(regex.errorString()).arg(regex.patternErrorOffset()).toLatin1().data());
        return 0;
    }

    Alias *alias = p->findAlias(name, p->rootGroup());
    if (alias)
    {
        luaL_error(L, tr("cannot create duplicate alias with name '%1'").arg(name).toLatin1().data());
        return 0;
    }

    alias = new Alias;

    alias->setName(name);
    alias->setPattern(pattern);
    alias->setContents(contents);
    alias->setSequence(qBound(1, (int)optionalNumber(L, 4, 1000), 100000));

    p->addAlias(alias);

    return alias;
}

int Engine::aliasGet(lua_State *L)
{
    Profile *p = profile(L);

    if (lua_isnone(L, 1))
    {
        lua_newtable(L);

        int i = 0;
        QList<Alias *> aliases = p->rootGroup()->sortedAliases();
        foreach (Alias *alias, aliases)
        {
            lua_pushnumber(L, ++i);
            luaW_push<Alias>(L, alias);
        }

        return 1;
    }

    luaW_push<Alias>(L, p->findAlias(checkString(L, 1)));
    return 1;
}

int Engine::aliasDelete(lua_State *L)
{
    Profile *p = profile(L);

    Alias *alias = luaW_check<Alias>(L, 1);

    bool result = p->deleteAlias(alias);

    lua_pushnil(L);
    lua_setmetatable(L, -2);

    lua_pushboolean(L, result);
    return 1;
}

int Engine::aliasFire(lua_State *L)
{
    Profile *p = profile(L);

    Alias *alias = luaW_check<Alias>(L, 1);
    Q_ASSERT(alias != 0);

    QString str(checkString(L, 2));

    if (!alias->regex().isValid())
    {
        lua_pushboolean(L, false);
        lua_pushstring(L, tr("Regex error: %1 (column %2)").arg(alias->regex().errorString()).arg(alias->regex().patternErrorOffset()).toLatin1().data());
        return 2;
    }

    if (alias->match(str))
    {
        saveCaptures(L, alias);

        Group *previousGroup = p->activeGroup();
        Q_ASSERT(previousGroup != 0);
        p->setActiveGroup(alias->group());

        int err = execute(L, alias->contents(), alias);
        if (err != 0)
        {
            lua_pushboolean(L, false);
            lua_pushstring(L, tr("Lua error %1: %2").arg(err).arg(lua_tostring(L, -2)).toLatin1().data());
            return 2;
        }

        p->setActiveGroup(previousGroup);
    }

    lua_pushboolean(L, true);
    return 1;
}

int Engine::aliasEnable(lua_State *L)
{
    return itemEnable(L, luaW_check<Alias>(L, 1));
}

int Engine::aliasProperty(lua_State *L)
{
    return itemProperty(L, luaW_check<Alias>(L, 1));
}

int Engine::aliasString(lua_State *L)
{
    return itemXml(L, luaW_check<Alias>(L, 1), false);
}

int Engine::aliasXml(lua_State *L)
{
    return itemXml(L, luaW_check<Alias>(L, 1), optionalBoolean(L, 2, true));
}


Event * Engine::eventNew(lua_State *L)
{
    QString name(checkString(L, 1));
    int reference = LUA_NOREF;

    luaL_checktype(L, 2, LUA_TFUNCTION);

    lua_pushvalue(L, 2);
    reference = luaL_ref(L, LUA_REGISTRYINDEX);

    EventMap *h = handlers(L);

    QList<Event *> events;
    if (h->contains(name))
    {
        events = h->value(name);
    }

    Event *event = new Event(console(L));
    event->setName(name);
    event->setReference(reference);
    event->setSequence(qBound(1, (int)optionalNumber(L, 3, 1000), 100000));
    events.append(event);

    qSort(events.begin(), events.end(), Event::lessSequence);
    h->insert(name, events);

    return event;
}

int Engine::eventList(lua_State *L)
{
    EventMap *h = handlers(L);

    if (lua_isnoneornil(L, 1))
    {
        lua_newtable(L);

        QStringList keys(h->keys());
        foreach (QString key, keys)
        {
            lua_newtable(L);

            int n = 1;
            QList<Event *> events(h->value(key));
            foreach (Event *event, events)
            {
                lua_pushinteger(L, n++);
                luaW_push<Event>(L, event);
                lua_settable(L, -3);
            }

            lua_setfield(L, -2, key.toLatin1().data());
        }

        return 1;
    }

    QString name(checkString(L, 1));
    if (!h->contains(name))
    {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    int n = 1;
    QList<Event *> events(h->value(name));
    foreach (Event *event, events)
    {
        lua_pushinteger(L, n++);
        luaW_push<Event>(L, event);
        lua_settable(L, -3);
    }

    return 1;
}

int Engine::eventUnregister(lua_State *L)
{
    if (lua_isuserdata(L, 1))
    {
        Event *event = luaW_check<Event>(L, 1);

        EventMap *h = handlers(L);

        QStringList keys(h->keys());
        foreach (QString key, keys)
        {
            QList<Event *> events(h->value(key));
            if (events.removeOne(event))
            {
                luaL_unref(L, LUA_REGISTRYINDEX, event->reference());

                if (events.isEmpty())
                {
                    h->remove(key);
                }
                else
                {
                    h->insert(key, events);
                }

                lua_pushnil(L);
                lua_setmetatable(L, -2);

                delete event;

                // TODO: Lua GC function call?

                lua_pushboolean(L, true);
                return 1;
            }
        }

        lua_pushboolean(L, false);
        return 1;
    }

    QString name(checkString(L, 1));

    EventMap *h = handlers(L);

    if (!h->contains(name))
    {
        lua_pushboolean(L, false);
        return 1;
    }

    int count = 0;
    QList<Event *> events(h->take(name));
    foreach (Event *e, events)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, e->reference());

        delete e;

        count++;
    }

    lua_pushnumber(L, count);
    return 1;
}

int Engine::eventRaise(lua_State *L)
{
    QString name(checkString(L, 1));

    EventMap *h = handlers(L);
    if (!h->contains(name))
    {
        lua_pushnil(L);
        return 1;
    }

    int count = 0;
    int failed = 0;

    QVariantList args;
    int argCount = lua_gettop(L);

    for (int n = 2; n <= argCount; n++)
    {
        args << checkVariant(L, n);
    }

    QList<Event *> events = h->value(name);
    foreach (Event *event, events)
    {
        if (callEventHandler(L, event, args) == 0)
        {
            count++;
        }
        else
        {
            failed++;
        }
    }

    lua_pushinteger(L, count);
    lua_pushinteger(L, failed);
    return 2;
}

int Engine::eventProperty(lua_State *L)
{
    return itemProperty(L, luaW_check<Event>(L, 1));
}

int Engine::eventString(lua_State *L)
{
    return itemXml(L, luaW_check<Event>(L, 1), false);
}

int Engine::eventXml(lua_State *L)
{
    return itemXml(L, luaW_check<Event>(L, 1), optionalBoolean(L, 2, true));
}


Group * Engine::groupNew(lua_State *L)
{
    Profile *p = profile(L);

    QString name(checkString(L, 1));
    bool enabled = optionalBoolean(L, 2, true);

    if (name.isEmpty())
    {
        luaL_error(L, tr("cannot create an unnamed group").toLatin1().data());
        return 0;
    }

    Group * group = p->findGroup(name, p->rootGroup());
    if (group)
    {
        luaL_error(L, tr("cannot create duplicate group with name '%1'").arg(name).toLatin1().data());
        return 0;
    }

    group = new Group;

    QString cleanPath(QDir::cleanPath(name));
    QStringList names(cleanPath.split('/'));

    group->setName(names.takeLast());
    group->enable(enabled);

    Group *active = p->activeGroup();
    Group *parent = p->findGroup(names.join('/'));
    if (parent)
    {
        p->setActiveGroup(parent);
    }

    p->addGroup(group);

    if (parent)
    {
        p->setActiveGroup(active);
    }

    return group;
}

int Engine::groupGet(lua_State *L)
{
    Profile *p = profile(L);

    if (lua_isnoneornil(L, 1))
    {
        luaW_push<Group>(L, p->activeGroup());
        return 1;
    }

    luaW_push<Group>(L, p->findGroup(checkString(L, 1)));
    return 1;
}

int Engine::groupDelete(lua_State *L)
{
    Profile *p = profile(L);

    Group *group = luaW_check<Group>(L, 1);

    bool result = p->deleteGroup(group);

    lua_pushnil(L);
    lua_setmetatable(L, -2);

    lua_pushboolean(L, result);
    return 1;
}

int Engine::groupEnable(lua_State *L)
{
    return itemEnable(L, luaW_check<Group>(L, 1));
}

int Engine::groupProperty(lua_State *L)
{
    return itemProperty(L, luaW_check<Group>(L, 1));
}

int Engine::groupString(lua_State *L)
{
    return itemXml(L, luaW_check<Group>(L, 1), false);
}

int Engine::groupXml(lua_State *L)
{
    return itemXml(L, luaW_check<Group>(L, 1), optionalBoolean(L, 2, true));
}

int Engine::groupAccelerators(lua_State *L)
{
    Group *group = luaW_check<Group>(L, 1);

    lua_newtable(L);
    int i = 0;
    foreach (Accelerator *accelerator, group->accelerators())
    {
        lua_pushnumber(L, ++i);
        luaW_push<Accelerator>(L, accelerator);
        lua_settable(L, -3);
    }

    return 1;
}

int Engine::groupAliases(lua_State *L)
{
    Group *group = luaW_check<Group>(L, 1);

    lua_newtable(L);
    int i = 0;
    foreach (Alias *alias, group->aliases())
    {
        lua_pushnumber(L, ++i);
        luaW_push<Alias>(L, alias);
        lua_settable(L, -3);
    }

    return 1;
}

int Engine::groupGroups(lua_State *L)
{
    Group *group = luaW_check<Group>(L, 1);

    lua_newtable(L);
    int i = 0;
    foreach (Group *child, group->groups())
    {
        lua_pushnumber(L, ++i);
        luaW_push<Group>(L, child);
        lua_settable(L, -3);
    }

    return 1;
}

int Engine::groupTimers(lua_State *L)
{
    Group *group = luaW_check<Group>(L, 1);

    lua_newtable(L);
    int i = 0;
    foreach (Timer *timer, group->timers())
    {
        lua_pushnumber(L, ++i);
        luaW_push<Timer>(L, timer);
        lua_settable(L, -3);
    }

    return 1;
}

int Engine::groupTriggers(lua_State *L)
{
    Group *group = luaW_check<Group>(L, 1);

    lua_newtable(L);
    int i = 0;
    foreach (Trigger *trigger, group->triggers())
    {
        lua_pushnumber(L, ++i);
        luaW_push<Trigger>(L, trigger);
        lua_settable(L, -3);
    }

    return 1;
}

int Engine::groupVariables(lua_State *L)
{
    Group *group = luaW_check<Group>(L, 1);

    lua_newtable(L);
    int i = 0;
    foreach (Variable *variable, group->variables())
    {
        lua_pushnumber(L, ++i);
        luaW_push<Variable>(L, variable);
        lua_settable(L, -3);
    }

    return 1;
}

Timer * Engine::timerNew(lua_State *L)
{
    Profile *p = profile(L);

    QString name(checkString(L, 1));
    QString contents(checkString(L, 3));

    if (name.isEmpty())
    {
        luaL_error(L, tr("cannot create an unnamed timer").toLatin1().data());
        return 0;
    }

    if (contents.isEmpty())
    {
        luaL_error(L, tr("cannot create timer without script").toLatin1().data());
        return 0;
    }

    QTime intervalTime;
    if (lua_isnumber(L, 2))
    {
        double secondsArg = checkNumber(L, 2);
        int seconds = (int)secondsArg;
        int milliseconds = (secondsArg - seconds) * 1000;
        int hours = seconds / 60 / 60;
        seconds -= hours * 60 * 60;
        int minutes = seconds / 60;
        seconds -= minutes * 60;

        intervalTime.setHMS(hours, minutes, seconds, milliseconds);
    }
    else
    {
        QString interval(checkString(L, 2));
        if (interval.isEmpty())
        {
            luaL_error(L, tr("cannot create timer with no interval").toLatin1().data());
            return 0;
        }

        QStringList timeFormats;
        timeFormats << "s" << "s.z" << "m:s.z" << "h:m:s.z";

        foreach (QString timeFormat, timeFormats)
        {
            intervalTime = QTime::fromString(interval, timeFormat);
            if (intervalTime.isValid())
            {
                break;
            }
        }
    }

    if (!intervalTime.isValid())
    {
        luaL_error(L, tr("cannot create timer with invalid interval").toLatin1().data());
        return 0;
    }

    Timer *timer = p->findTimer(name, p->rootGroup());
    if (timer)
    {
        luaL_error(L, tr("cannot create duplicate timer with name '%1'").arg(name).toLatin1().data());
        return 0;
    }

    timer = new Timer;

    timer->setName(name);
    timer->setInterval(intervalTime);
    timer->setContents(contents);
    timer->enable(optionalBoolean(L, 4, true));
    timer->setOnce(optionalBoolean(L, 5, false));

    p->addTimer(timer);

    return timer;
}

int Engine::timerGet(lua_State *L)
{
    Profile *p = profile(L);

    if (lua_isnone(L, 1))
    {
        lua_newtable(L);

        int i = 0;
        QList<Timer *> timers = p->rootGroup()->sortedTimers();
        foreach (Timer *timer, timers)
        {
            lua_pushnumber(L, ++i);
            luaW_push<Timer>(L, timer);
        }

        return 1;
    }

    luaW_push<Timer>(L, p->findTimer(checkString(L, 1)));
    return 1;
}

int Engine::timerDelete(lua_State *L)
{
    Profile *p = profile(L);

    Timer *timer = luaW_check<Timer>(L, 1);

    bool result = p->deleteTimer(timer);

    lua_pushnil(L);
    lua_setmetatable(L, -2);

    lua_pushboolean(L, result);
    return 1;
}

int Engine::timerFire(lua_State *L)
{
    Profile *p = profile(L);

    Timer *timer = luaW_check<Timer>(L, 1);
    Q_ASSERT(timer != 0);

    Group *previousGroup = p->activeGroup();
    Q_ASSERT(previousGroup != 0);
    p->setActiveGroup(timer->group());

    int err = execute(L, timer->contents(), timer);
    if (err != 0)
    {
        lua_pushboolean(L, false);
        lua_pushstring(L, tr("Lua error %1: %2").arg(err).arg(lua_tostring(L, -2)).toLatin1().data());
        return 2;
    }

    p->setActiveGroup(previousGroup);

    lua_pushboolean(L, true);
    return 1;
}

int Engine::timerEnable(lua_State *L)
{
    return itemEnable(L, luaW_check<Timer>(L, 1));
}

int Engine::timerProperty(lua_State *L)
{
    return itemProperty(L, luaW_check<Timer>(L, 1));
}

int Engine::timerString(lua_State *L)
{
    return itemXml(L, luaW_check<Timer>(L, 1), false);
}

int Engine::timerXml(lua_State *L)
{
    return itemXml(L, luaW_check<Timer>(L, 1), optionalBoolean(L, 2, true));
}


Trigger * Engine::triggerNew(lua_State *L)
{
    Profile *p = profile(L);

    QString name(checkString(L, 1));
    QString pattern(checkString(L, 2));
    QString contents(checkString(L, 3));

    if (name.isEmpty())
    {
        luaL_error(L, tr("cannot create an unnamed trigger").toLatin1().data());
        return 0;
    }

    if (pattern.isEmpty())
    {
        luaL_error(L, tr("cannot create trigger with empty pattern").toLatin1().data());
        return 0;
    }

    if (contents.isEmpty())
    {
        luaL_error(L, tr("cannot create trigger without script").toLatin1().data());
        return 0;
    }

    QRegularExpression regex(pattern);
    if (!regex.isValid())
    {
        luaL_error(L, tr("invalid regular expression: %1 (column %2)").arg(regex.errorString()).arg(regex.patternErrorOffset()).toLatin1().data());
        return 0;
    }

    Trigger *trigger = p->findTrigger(name, p->rootGroup());
    if (trigger)
    {
        luaL_error(L, tr("cannot create duplicate trigger with name '%1'").arg(name).toLatin1().data());
        return 0;
    }

    trigger = new Trigger;

    trigger->setName(name);
    trigger->setPattern(pattern);
    trigger->setContents(contents);
    trigger->setSequence(qBound(1, (int)optionalNumber(L, 4, 1000), 100000));

    p->addTrigger(trigger);

    return trigger;
}

int Engine::triggerGet(lua_State *L)
{
    Profile *p = profile(L);

    if (lua_isnone(L, 1))
    {
        lua_newtable(L);

        int i = 0;
        QList<Trigger *> triggers = p->rootGroup()->sortedTriggers();
        foreach (Trigger *trigger, triggers)
        {
            lua_pushnumber(L, ++i);
            luaW_push<Trigger>(L, trigger);
        }

        return 1;
    }

    luaW_push<Trigger>(L, p->findTrigger(checkString(L, 1)));
    return 1;
}

int Engine::triggerTest(lua_State *L)
{
    Console *c = console(L);

    QString str(checkString(L, 1));

    lua_pushboolean(L, c->processTriggers(str));

    return 1;
}

int Engine::triggerDelete(lua_State *L)
{
    Profile *p = profile(L);

    Trigger *trigger = luaW_check<Trigger>(L, 1);

    bool result = p->deleteTrigger(trigger);

    lua_pushnil(L);
    lua_setmetatable(L, -2);

    lua_pushboolean(L, result);
    return 1;
}

int Engine::triggerFire(lua_State *L)
{
    Profile *p = profile(L);

    Trigger *trigger = luaW_check<Trigger>(L, 1);
    Q_ASSERT(trigger != 0);

    QString str(checkString(L, 2));

    if (!trigger->regex().isValid())
    {
        lua_pushboolean(L, false);
        lua_pushstring(L, tr("Regex error: %1 (column %2)").arg(trigger->regex().errorString()).arg(trigger->regex().patternErrorOffset()).toLatin1().data());
        return 2;
    }

    if (trigger->match(str))
    {
        saveCaptures(L, trigger);

        Group *previousGroup = p->activeGroup();
        Q_ASSERT(previousGroup != 0);
        p->setActiveGroup(trigger->group());

        int err = execute(L, trigger->contents(), trigger);
        if (err != 0)
        {
            lua_pushboolean(L, false);
            lua_pushstring(L, tr("Lua error %1: %2").arg(err).arg(lua_tostring(L, -2)).toLatin1().data());
            return 2;
        }

        p->setActiveGroup(previousGroup);
    }

    lua_pushboolean(L, true);
    return 1;
}

int Engine::triggerEnable(lua_State *L)
{
    return itemEnable(L, luaW_check<Trigger>(L, 1));
}

int Engine::triggerProperty(lua_State *L)
{
    return itemProperty(L, luaW_check<Trigger>(L, 1));
}

int Engine::triggerString(lua_State *L)
{
    return itemXml(L, luaW_check<Trigger>(L, 1), false);
}

int Engine::triggerXml(lua_State *L)
{
    return itemXml(L, luaW_check<Trigger>(L, 1), optionalBoolean(L, 2, true));
}


Variable * Engine::variableNew(lua_State *L)
{
    Profile *p = profile(L);

    QString name(checkString(L, 1));
    QString contents(checkString(L, 2));

    if (name.isEmpty())
    {
        luaL_error(L, tr("cannot create an unnamed variable").toLatin1().data());
        return 0;
    }

    if (contents.isEmpty())
    {
        luaL_error(L, tr("cannot create variable with no value").toLatin1().data());
        return 0;
    }

    Variable *variable = p->findVariable(name);
    if (variable)
    {
        luaL_error(L, tr("cannot create duplicate variable with name '%1'").arg(name).toLatin1().data());
        return 0;
    }

    variable = new Variable;

    variable->setName(name);
    variable->setContents(contents);

    p->addVariable(variable);

    return variable;
}

int Engine::variableGet(lua_State *L)
{
    Profile *p = profile(L);

    if (lua_isnone(L, 1))
    {
        lua_newtable(L);

        int i = 0;
        QList<Variable *> variables = p->rootGroup()->sortedVariables();
        foreach (Variable *variable, variables)
        {
            lua_pushnumber(L, ++i);
            luaW_push<Variable>(L, variable);
        }

        return 1;
    }

    luaW_push<Variable>(L, p->findVariable(checkString(L, 1)));
    return 1;
}

int Engine::variableDelete(lua_State *L)
{
    Profile *p = profile(L);

    Variable *variable = luaW_check<Variable>(L, 1);

    bool result = p->deleteVariable(variable);

    lua_pushnil(L);
    lua_setmetatable(L, -2);

    lua_pushboolean(L, result);
    return 1;
}

int Engine::variableProperty(lua_State *L)
{
    return itemProperty(L, luaW_check<Variable>(L, 1));
}

int Engine::variableString(lua_State *L)
{
    return itemXml(L, luaW_check<Variable>(L, 1), false);
}

int Engine::variableXml(lua_State *L)
{
    return itemXml(L, luaW_check<Variable>(L, 1), optionalBoolean(L, 2, true));
}


int Engine::colorNote(lua_State *L)
{
    Console *c = console(L);

    int numArgs = lua_gettop(L);
    for (int n = 1; n <= numArgs; n += 3)
    {
        if (n + 2 < numArgs)
        {
            c->colorTell(checkString(L, n), checkString(L, n + 1), checkString(L, n + 2));
        }
        else
        {
            c->colorNote(checkString(L, n), checkString(L, n + 1), checkString(L, n + 2));
        }
    }

    return 0;
}

int Engine::colorTell(lua_State *L)
{
    Console *c = console(L);

    int numArgs = lua_gettop(L);
    for (int n = 1; n <= numArgs; n += 3)
    {
        c->colorTell(checkString(L, n), checkString(L, n + 1), checkString(L, n + 2));
    }

    return 0;
}

int Engine::note(lua_State *L)
{
    Console *c = console(L);
    Profile *p = profile(L);

    c->colorNote(p->noteForeground(), p->noteBackground(), concatArgs(L));

    return 0;
}

int Engine::tell(lua_State *L)
{
    Console *c = console(L);
    Profile *p = profile(L);

    c->colorTell(p->noteForeground(), p->noteBackground(), concatArgs(L));

    return 0;
}

int Engine::print(lua_State *L)
{
    Console *c = console(L);
    Profile *p = profile(L);

    c->colorNote(p->noteForeground(), QColor(), concatArgs(L, " "));

    return 0;
}

int Engine::hyperlink(lua_State *L)
{
    Console *c = console(L);

    QColor fg(checkString(L, 1));
    QColor bg(checkString(L, 2));
    QString text(checkString(L, 3));
    QString hint(checkString(L, 4));

    QVariant link;
    if (lua_isstring(L, 5))
    {
        QString str(checkString(L, 5));
        QRegularExpression pattern("^(https?|mailto)://.+");
        QRegularExpressionMatch m(pattern.match(str));
        if (m.hasMatch())
        {
            QUrl url(QUrl::fromUserInput(str));
            if (url.isValid())
            {
                link = url;
            }
        }

        if (!link.isValid())
        {
            link = QUrl(QString("alias://%1").arg(str));
        }
    }
    else if (lua_isfunction(L, 5))
    {
        lua_pushvalue(L, 5);
        link = QUrl(QString("lua://function:%1").arg(luaL_ref(L, LUA_REGISTRYINDEX)));
    }
    else
    {
        return luaL_error(L, tr("link must be an alias, a URL, or a Lua function handle").toLatin1().data());
    }

    lua_pushnumber(L, c->hyperlink(text, fg, bg, new Hyperlink(hint, link)));

    return 1;
}

int Engine::send(lua_State *L)
{
    Console *c = console(L);

    bool echo = true;
    int n = lua_gettop(L);
    if (lua_isboolean(L, n))
    {
        echo = optionalBoolean(L, n, echo);
        lua_pop(L, 1);
        n = n - 1;
    }

    for (int cmd = 1; cmd <= n; cmd++)
    {
        c->send(checkString(L, cmd), echo);
    }

    return 0;
}

int Engine::sendAlias(lua_State *L)
{
    Console *c = console(L);

    int n = lua_gettop(L);
    for (int cmd = 1; cmd <= n; cmd++)
    {
        c->commandEntered(checkString(L, cmd));
    }

    return 0;
}

int Engine::sendGmcp(lua_State *L)
{
    Console *c = console(L);

    QString msg(checkString(L, 1));

    bool result = false;
    if (!lua_isnone(L, 2))
    {
        result = c->sendGmcp(msg, checkString(L, 2));
    }
    else
    {
        result = c->sendGmcp(msg);
    }

    lua_pushboolean(L, result);
    return 1;
}

int Engine::simulate(lua_State *L)
{
    Console *c = console(L);

    c->dataReceived(checkString(L, 1).toLocal8Bit());

    return 0;
}

int Engine::openLog(lua_State *L)
{
    Console *c = console(L);

    QString result(c->startLog(optionalString(L, 1, QString()), optionalBoolean(L, 2, true)));
    if (!result.isEmpty())
    {
        return luaL_error(L, result.toLatin1().data());
    }

    lua_pushstring(L, c->logInfo().canonicalFilePath().toLatin1().data());
    return 1;
}

int Engine::logNote(lua_State *L)
{
    Console *c = console(L);

    c->logNote(concatArgs(L));

    return 0;
}

int Engine::closeLog(lua_State *L)
{
    Console *c = console(L);

    c->finishLog();

    return 0;
}

int Engine::isLogging(lua_State *L)
{
    Console *c = console(L);

    lua_pushboolean(L, c->isLogging());

    return 1;
}

int Engine::pauseLog(lua_State *L)
{
    Console *c = console(L);

    c->pauseLog(optionalBoolean(L, 1, true));

    return 0;
}

int Engine::setVariable(lua_State *L)
{
    Profile *p = profile(L);

    QVariant val;
    if (lua_isnumber(L, 2))
    {
        val = QVariant(lua_tonumber(L, 2));
        if (!val.isValid())
        {
            return luaL_argerror(L, 2, lua_pushfstring(L, tr("conversion to double failed").toLatin1().data()));
        }
    }
    else if (lua_isboolean(L, 2))
    {
        val = QVariant(lua_toboolean(L, 2) != 0);
        if (!val.isValid())
        {
            return luaL_argerror(L, 2, lua_pushfstring(L, tr("conversion to boolean failed").toLatin1().data()));
        }
    }
    else
    {
        val = QVariant(checkString(L, 2));
    }

    p->setVariable(checkString(L, 1), val);

    return 0;
}

int Engine::getVariable(lua_State *L)
{
    Profile *p = profile(L);

    QVariant result(p->getVariable(checkString(L, 1)));
    if (result.isNull())
    {
        lua_pushnil(L);
    }
    else
    {
        switch (result.type())
        {
        case QVariant::Double:
            lua_pushnumber(L, result.toDouble());
            break;

        case QVariant::Bool:
            lua_pushboolean(L, result.toBool());
            break;

        default:
            lua_pushstring(L, result.toString().toLatin1());
            break;
        }
    }

    return 1;
}

int Engine::deleteVariable(lua_State *L)
{
    Profile *p = profile(L);

    Variable *variable = p->findVariable(checkString(L, 1));
    bool result = variable != 0;
    if (variable != 0)
    {
        result = p->deleteVariable(variable);
    }

    lua_pushboolean(L, result);
    return 1;
}

int Engine::jsonDecode(lua_State *L)
{
    if (lua_isnoneornil(L, 1))
    {
        lua_pushnil(L);
    }
    else
    {
        QString data(checkString(L, 1));
        QJsonDocument doc(QJsonDocument::fromJson(data.toUtf8()));

        pushVariant(L, doc);
    }

    return 1;
}

int Engine::jsonEncode(lua_State *L)
{
    if (lua_isnoneornil(L, 1))
    {
        lua_pushnil(L);
    }
    else
    {
        QVariant var(checkVariant(L, 1));
        if (var.isNull())
        {
            lua_pushnil(L);
        }
        else
        {
            QJsonDocument json(QJsonDocument::fromVariant(var));
            lua_pushstring(L, json.toJson(QJsonDocument::Compact));
        }
    }

    return 1;
}

int Engine::isConnected(lua_State *L)
{
    Console *c = console(L);

    lua_pushboolean(L, c->isConnected());

    return 1;
}

int Engine::connect(lua_State *L)
{
    Console *c = console(L);

    c->connectToServer();

    return 0;
}

int Engine::disconnect(lua_State *L)
{
    Console *c = console(L);

    c->disconnectFromServer();

    return 0;
}

int Engine::playSound(lua_State *L)
{
    Console *c = console(L);
    QString filename(checkString(L, 1));

    QUrl url(filename);
    if (!url.isValid())
    {
        return luaL_error(L, tr("invalid URL (%1): %2").arg(filename).arg(url.errorString()).toLatin1().data());
    }

    QString result(c->playMedia(QMediaContent(url)));
    if (!result.isEmpty())
    {
        return luaL_error(L, tr("failed to play media: %1").arg(result).toLatin1().data());
    }

    return 0;
}
