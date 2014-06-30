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


#include <QApplication>
#include <QMessageBox>
#include "engine.h"
#include "LuaBridge.h"
#include "logging.h"
#include "console.h"
#include "event.h"
#include "matchable.h"
#include "profile.h"
#include "profileitem.h"

using namespace luabridge;


inline bool lua_isarray(lua_State *L, int index)
{
    if (!lua_istable(L, index))
    {
        return false;
    }

    lua_pushvalue(L, index);
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

namespace luabridge
{

template <>
struct Stack <QString>
{
    static void push(lua_State *L, QString value)
    {
        lua_pushstring(L, qPrintable(value));
    }

    static QString get(lua_State *L, int index)
    {
        return luaL_checkstring(L, index);
    }
};

template <>
struct Stack <QVariant>
{
    static void push(lua_State *L, QVariant value)
    {
        if (!value.isValid())
        {
            lua_pushnil(L);
            return;
        }

        switch (value.userType())
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
            lua_pushstring(L, qPrintable(regex.pattern()));
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
                lua_pushstring(L, qPrintable(regex.errorString()));
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
            lua_pushstring(L, qPrintable(font.family()));
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
                push(L, json.array());
            }
            else if (json.isObject())
            {
                push(L, json.object());
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
                push(L, element);
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
                lua_pushstring(L, qPrintable(key));
                push(L, json.value(key));
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
                push(L, json.toArray());
            }
            else if (json.isObject())
            {
                lua_newtable(L);
                push(L, json.toObject());
            }
            else
            {
                push(L, json.toVariant());
            }
        }
            break;

        case QMetaType::QVariantList:
        {
            QVariantList list(value.toList());

            int n = 0;
            lua_newtable(L);
            foreach (QVariant v, list)
            {
                lua_pushnumber(L, ++n);
                push(L, v);
                lua_settable(L, -3);
            }
        }
            break;

        case QMetaType::QVariantMap:
        {
            QVariantMap map(value.toMap());

            lua_newtable(L);
            foreach (QString key, map.keys())
            {
                lua_pushstring(L, qPrintable(key));
                push(L, map.value(key));
                lua_settable(L, -3);
            }
        }
            break;

        default:
            lua_pushstring(L, qPrintable(value.toString()));
            break;
        }
    }

    static QVariant get(lua_State *L, int index)
    {
        QVariant v;
        if (lua_isnumber(L, index))
        {
            v = QVariant(luaL_checknumber(L, index));
        }
        else if (lua_isboolean(L, index))
        {
            v = QVariant(lua_toboolean(L, index) != 0);
        }
        else if (lua_isarray(L, index))
        {
            QVariantList list;

            lua_pushvalue(L, index);
            lua_pushnil(L);

            while (lua_next(L, -2))
            {
                lua_pushvalue(L, -2);

                list.insert(lua_tonumber(L, -1), get(L, -2));

                lua_pop(L, 2);
            }

            lua_pop(L, 1);

            v = list;
        }
        else if (lua_istable(L, index))
        {
            QVariantMap map;

            lua_pushvalue(L, index);
            lua_pushnil(L);

            while (lua_next(L, -2))
            {
                lua_pushvalue(L, -2);

                map.insert(lua_tostring(L, -1), get(L, -2));

                lua_pop(L, 2);
            }

            lua_pop(L, 1);

            v = map;
        }
        else if (!lua_isnil(L, index))
        {
            v = QVariant(luaL_checkstring(L, index));
        }

        return v;
    }
};

}


Engine::Engine(QObject *parent) :
    QObject(parent)
{
    m_GMCP = false;
}

Engine::~Engine()
{
}

void Engine::initialize(Console *c)
{
    lua_atpanic(m_global, Engine::panic);

    getGlobalNamespace(m_global)
        .beginNamespace("gmcp")
            .addVariable("enabled", &m_GMCP, false)
        .endNamespace()
        .addCFunction("print", Engine::print)
        .addCFunction("Send", Engine::send)
        .addCFunction("SendAlias", Engine::sendAlias)
        .addCFunction("SendGmcp", Engine::sendGmcp)
        .addCFunction("JsonDecode", Engine::jsonDecode)
        .addCFunction("JsonEncode", Engine::jsonEncode)
        .addCFunction("GetVariable", Engine::getVariable)
        .addCFunction("SetVariable", Engine::setVariable)
        .addCFunction("DeleteVariable", Engine::deleteVariable)
        .addCFunction("IsConnected", Engine::isConnected)
        .addCFunction("Connect", Engine::connectRemote)
        .addCFunction("Disconnect", Engine::disconnectRemote)
        .addCFunction("Version", Engine::version)
        .addCFunction("RaiseEvent", Engine::raiseEvent)
        .addCFunction("RegisterEvent", Engine::registerEvent)
        .addCFunction("UnregisterEvent", Engine::unregisterEvent);

    lua_settop(m_global, 0);

    loadResource(m_global, ":/lua/inspect");
    loadResource(m_global, ":/lua/io_exists");
    loadResource(m_global, ":/lua/string_commas");
    loadResource(m_global, ":/lua/string_explode");
    loadResource(m_global, ":/lua/string_join");
    loadResource(m_global, ":/lua/string_split");
    loadResource(m_global, ":/lua/string_title");
    loadResource(m_global, ":/lua/string_trim");
    loadResource(m_global, ":/lua/string_wrap");
    loadResource(m_global, ":/lua/table_contains");
    loadResource(m_global, ":/lua/table_copy");
    loadResource(m_global, ":/lua/table_ordered");
    loadResource(m_global, ":/lua/table_print");
    loadResource(m_global, ":/lua/table_size");

    setRegistryData("CONSOLE", (void *)c);
    setRegistryData("ENGINE", (void *)this);
    setRegistryData("HANDLERS", (void *)(new EventMap()));
}

void Engine::setRegistryData(const QString &name, void *data)
{
    if (data)
    {
        lua_pushlightuserdata(m_global, data);
    }
    else
    {
        lua_pushnil(m_global);
    }

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
    qCCritical(MUDDER_SCRIPT) << msg;

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
        qCCritical(MUDDER_SCRIPT) << "No active console.";
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

bool Engine::execute(const QString &code, const QObject *item, const QVariantList &args)
{
    if (code.isEmpty())
    {
        return true;
    }

    if (!m_global)
    {
        qCCritical(MUDDER_SCRIPT) << "No Lua execution unit found.";
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
    if (err != LUA_OK)
    {
        clearArguments();
        clearCaptures();

        error(m_global, tr("Compile error"));
        return false;
    }

    saveCaptures(qobject_cast<const Matchable *>(item));
    saveArguments(args);

    err = m_global.pcall(args.count(), LUA_MULTRET);

    // Something didn't work, print it out
    if (err != LUA_OK)
    {
        error(m_global, tr("Run-time error"));
    }

    clearArguments();
    clearCaptures();

    lua_settop(m_global, 0);
    m_chunk.clear();

    setRegistryData("CALLER", 0);

    return err == LUA_OK;
}

bool Engine::execute(int id, const QObject *item, const QVariantList &args)
{
    if (id < 1)
    {
        return false;
    }

    if (!m_global)
    {
        qCCritical(MUDDER_SCRIPT) << "No Lua execution unit found.";
        return false;
    }

    // Start with an empty stack
    lua_settop(m_global, 0);

    // Give access to the caller object
    setRegistryData("CALLER", (void *)item);

    lua_rawgeti(m_global, LUA_REGISTRYINDEX, id);

    saveCaptures(qobject_cast<const Matchable *>(item));
    saveArguments(args);

    int err = m_global.pcall(args.count(), LUA_MULTRET);

    // Something didn't work, print it out
    if (err != LUA_OK)
    {
        error(m_global, tr("Run-time error"));
    }

    lua_settop(m_global, 0);

    clearArguments();
    clearCaptures();

    setRegistryData("CALLER", 0);

    return err == LUA_OK;
}

void Engine::processEvents(const QString &name, const QVariantList &args)
{
    if (!m_global)
    {
        return;
    }

    EventMap *h = registryData<EventMap>("HANDLERS", m_global);

    QList<Event *> events(h->value(name));
    foreach (Event *event, events)
    {
        event->execute(this, args);
    }
}

void Engine::saveArguments(const QVariantList &args)
{
    LuaRef a(newTable(m_global));

    int n = 0;
    foreach (QVariant arg, args)
    {
        a[++n] = arg;
        push(m_global, arg);
    }

    setGlobal(m_global, a, "args");
}

void Engine::saveCaptures(const Matchable * const item)
{
    if (!item || !item->lastMatch())
    {
        return;
    }

    LuaRef m(newTable(m_global));

    for (int capture = 0; capture <= item->lastMatch()->lastCapturedIndex(); capture++)
    {
        m[capture] = qPrintable(item->lastMatch()->captured(capture));
    }

    foreach (QString name, item->regex().namedCaptureGroups())
    {
        if (!name.isEmpty())
        {
            m[qPrintable(name)] = qPrintable(item->lastMatch()->captured(name));
        }
    }

    setGlobal(m_global, m, "matches");
}

void Engine::clearArguments()
{
    lua_pushnil(m_global);
    lua_setglobal(m_global, "args");
}

void Engine::clearCaptures()
{
    lua_pushnil(m_global);
    lua_setglobal(m_global, "matches");
}

int Engine::print(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);
    c->printInfo(concatArgs(L, " "));

    return 0;
}

int Engine::send(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);

    bool echo = true;
    int n = lua_gettop(L);
    if (lua_isboolean(L, n))
    {
        echo = LuaRef::fromStack(L, n);
        lua_pop(L, 1);
        n = n - 1;
    }

    for (int cmd = 1; cmd <= n; cmd++)
    {
        c->send(luaL_checkstring(L, cmd), echo);
    }

    return 0;
}

int Engine::sendAlias(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);

    int n = lua_gettop(L);
    for (int cmd = 1; cmd <= n; cmd++)
    {
        c->sendAlias(luaL_checkstring(L, cmd));
    }

    return 0;
}

int Engine::sendGmcp(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);

    QString msg(luaL_checkstring(L, 1));

    bool result = false;
    if (!lua_isnone(L, 2))
    {
        result = c->sendGmcp(msg, LuaRef::fromStack(L, 2));
    }
    else
    {
        result = c->sendGmcp(msg);
    }

    push(L, result);
    return 1;
}

int Engine::jsonDecode(lua_State *L)
{
    if (lua_isnoneornil(L, 1))
    {
        lua_pushnil(L);
    }
    else if (lua_isnumber(L, 1))
    {
        lua_pushnumber(L, luaL_checknumber(L, 1));
    }
    else if (lua_isboolean(L, 1))
    {
        lua_pushboolean(L, lua_toboolean(L, 1));
    }
    else
    {
        QString data(luaL_checkstring(L, 1));
        QJsonDocument doc(QJsonDocument::fromJson(data.toUtf8()));

        if (doc.isEmpty())
        {
            lua_pushstring(L, qPrintable(data));
        }
        else
        {
            push(L, QVariant(doc));
        }
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
        QVariant var(LuaRef::fromStack(L, 1).cast<QVariant>());
        if (var.isNull())
        {
            lua_pushnil(L);
        }
        else
        {
            switch (var.type())
            {
            case QVariant::Map:
            case QVariant::List:
            case QVariant::StringList:
            {
                QJsonDocument json(QJsonDocument::fromVariant(var));
                lua_pushstring(L, json.toJson(QJsonDocument::Compact));
            }
                break;

            default:
                push(L, var);
                break;
            }
        }
    }

    return 1;
}

int Engine::getVariable(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);
    Profile *p = c->profile();

    push(L, p->getVariable(luaL_checkstring(L, 1)));

    return 1;
}

int Engine::setVariable(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);
    Profile *p = c->profile();

    if (lua_isnoneornil(L, 2))
    {
        push(L, p->deleteVariable(luaL_checkstring(L, 1)));

        return 1;
    }

    QVariant val;
    if (lua_isnumber(L, 2))
    {
        val = QVariant(lua_tonumber(L, 2));
        if (!val.isValid())
        {
            return luaL_argerror(L, 2, lua_pushfstring(L, qPrintable(tr("conversion to double failed"))));
        }
    }
    else if (lua_isboolean(L, 2))
    {
        val = QVariant(lua_toboolean(L, 2) != 0);
        if (!val.isValid())
        {
            return luaL_argerror(L, 2, lua_pushfstring(L, qPrintable(tr("conversion to boolean failed"))));
        }
    }
    else
    {
        val = QVariant(luaL_checkstring(L, 2));
    }

    push(L, p->setVariable(luaL_checkstring(L, 1), val));

    return 1;
}

int Engine::deleteVariable(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);
    Profile *p = c->profile();

    push(L, p->deleteVariable(luaL_checkstring(L, 1)));

    return 1;
}

int Engine::isConnected(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);

    push(L, c->isConnected());

    return 1;
}

int Engine::connectRemote(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);

    c->connectToServer();

    return 0;
}

int Engine::disconnectRemote(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);

    c->disconnectFromServer();

    return 0;
}

int Engine::version(lua_State *L)
{
    push(L, qPrintable(QApplication::applicationVersion()));

    return 1;
}

int Engine::raiseEvent(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);
    Engine *e = registryObject<Engine>("ENGINE", L);

    QString name(luaL_checkstring(L, 1));

    int numArgs = lua_gettop(L);
    QVariantList args;
    for (int n = 2; n <= numArgs; n++)
    {
        args << LuaRef::fromStack(L, n).cast<QVariant>();
    }

    c->processEvents(name, args);
    e->processEvents(name, args);

    return 0;
}

int Engine::registerEvent(lua_State *L)
{
    Console *c = registryObject<Console>("CONSOLE", L);
    EventMap *h = registryData<EventMap>("HANDLERS", L);

    QString name(luaL_checkstring(L, 1));

    luaL_checktype(L, 2, LUA_TFUNCTION);

    lua_pushvalue(L, 2);
    int reference = luaL_ref(L, LUA_REGISTRYINDEX);

    QList<Event *> events(h->value(name));

    Event *event = new Event(c);
    event->setPattern(name);
    event->setReference(reference);
    event->setSequence(qBound(1, (int)luaL_optnumber(L, 3, 1000), 100000));
    events.append(event);

    qSort(events.begin(), events.end());
    h->insert(name, events);

    push(L, reference);

    return 1;
}

int Engine::unregisterEvent(lua_State *L)
{
    EventMap *h = registryData<EventMap>("HANDLERS", L);

    if (lua_isnumber(L, 1))
    {
        int reference = LuaRef::fromStack(L, 1);

        foreach (QString name, h->keys())
        {
            QList<Event *> events(h->value(name));
            foreach (Event *event, events)
            {
                if (event->reference() == reference)
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, reference);

                    events.removeOne(event);
                    h->insert(name, events);

                    delete event;

                    lua_pushboolean(L, true);
                    return 1;
                }
            }
        }

        lua_pushboolean(L, false);
        return 1;
    }

    QString name(luaL_checkstring(L, 1));

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

void Engine::enableGMCP(bool flag)
{
    if (m_GMCP != flag)
    {
        m_GMCP = flag;

        Console *c = registryObject<Console>("CONSOLE", m_global);
        if (c)
        {
            c->printInfo(m_GMCP?tr("GMCP enabled."):tr("GMCP disabled."));
        }
    }
}

void Engine::handleGMCP(const QString &name, const QString &args)
{
    QStringList modules(name.split('.', QString::SkipEmptyParts));
    if (modules.isEmpty())
    {
        return;
    }

    LuaRef gmcp = getGlobal(m_global, "gmcp");

    LuaRef data(gmcp);
    for (int n = 0; n < modules.count() - 1; n++)
    {
        QString key(modules.at(n));
        LuaRef val(data[qPrintable(key)]);
        if (val.isNil())
        {
            data[qPrintable(key)] = newTable(m_global);
        }
        data = LuaRef(data[qPrintable(key)]);
    }

    QVariantList varArgs;
    varArgs << name;

    QJsonDocument doc(QJsonDocument::fromJson(args.toUtf8()));
    if (doc.isEmpty())
    {
        varArgs << qPrintable(args);
        data[qPrintable(modules.last())] = qPrintable(args);
    }
    else
    {
        varArgs << QVariant(doc);
        data[qPrintable(modules.last())] = QVariant(doc);
    }

    processEvents("onGMCP", varArgs);
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
        QString err(tr("failed to load resource '%1': %2").arg(resource).arg(lua_tostring(L, -1)));
        qCCritical(MUDDER_SCRIPT) << err;
        luaL_error(L, qPrintable(err));
    }

    return ret;
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
            QString err(tr("'%1' must return a string to be concatenated").arg("tostring"));
            qCCritical(MUDDER_SCRIPT) << err;
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
