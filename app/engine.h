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

#ifndef ENGINE_H
#define ENGINE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QMap>
#include <QObject>
#include <QRegularExpressionMatch>
#include <QString>
#include "event.h"

extern "C"
{
    #include <src/lua.h>
    #include <src/lualib.h>
    #include <src/lauxlib.h>
}

class Accelerator;
class Alias;
class Console;
class Group;
class Matchable;
class Profile;
class ProfileItem;
class Timer;
class Trigger;
class Variable;

class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = 0);
    ~Engine();

    void initialize(Console *c, const QString &script = "");

    void registerEverything(lua_State *L);
    int loadResource(lua_State *L, const QString &resource);

    static bool compile(const QString &code, const QString &what, QString *err = 0);
    bool execute(const QString &code, QObject *item = 0);
    bool execute(int id, QObject *item = 0);
    static int execute(lua_State *L, const QString &code, QObject *item = 0);
    void processEvent(const QString &name, const QVariantList &args = QVariantList());

    void saveCaptures(const Matchable * const item);
    static void saveCaptures(lua_State *L, const Matchable * const item);

    static int typeError(lua_State *L, int narg, const char * tname);
    static void tagError(lua_State *L, int narg, int tag);

    static QString checkString(lua_State *L, int arg);
    static double checkNumber(lua_State *L, int arg);
    static int checkInteger(lua_State *L, int arg);
    static QVariant checkVariant(lua_State *L, int arg, int type = QVariant::Invalid);

    static QString optionalString(lua_State *L, int arg, const QString &def);
    static lua_Number optionalNumber(lua_State *L, int arg, lua_Number def);
    static bool optionalBoolean(lua_State *L, int arg, bool def);

    static QString concatArgs(lua_State *L, const QString &delimiter = "", const int first = 1);
    static void pushVariant(lua_State *L, const QVariant &value);
    static void pushXml(lua_State *L, ProfileItem *item, bool format = false);
    static void makeTableItem(lua_State *L, const QString &name, const QVariant &value);

    static int panic(lua_State *L);
    void error(lua_State *L, const QString &event);
    void findTraceback(lua_State *L);

    static Console * console(lua_State *L);
    static Profile * profile(lua_State *L);
    static QObject * caller(lua_State *L);
    static EventMap * handlers(lua_State *L);

    static int setProperty(lua_State *L, QObject *obj, int nameArg, int valueArg);
    static int getProperty(lua_State *L, QObject *obj, int nameArg);

    static int profileActiveGroup(lua_State *L);
    static int profileProperty(lua_State *L);
    static int profileImportXml(lua_State *L);

    static int itemEnable(lua_State *L, ProfileItem *item);
    static int itemProperty(lua_State *L, ProfileItem *item);
    static int itemString(lua_State *L, ProfileItem *item);
    static int itemXml(lua_State *L, ProfileItem *item, bool format = false);

    static Accelerator * acceleratorNew(lua_State *L);
    static int acceleratorGet(lua_State *L);
    static int acceleratorDelete(lua_State *L);
    static int acceleratorFire(lua_State *L);
    static int acceleratorEnable(lua_State *L);
    static int acceleratorProperty(lua_State *L);
    static int acceleratorString(lua_State *L);
    static int acceleratorXml(lua_State *L);

    static Alias * aliasNew(lua_State *L);
    static int aliasGet(lua_State *L);
    static int aliasDelete(lua_State *L);
    static int aliasFire(lua_State *L);
    static int aliasEnable(lua_State *L);
    static int aliasProperty(lua_State *L);
    static int aliasString(lua_State *L);
    static int aliasXml(lua_State *L);

    static Event * eventNew(lua_State *L);
    static int eventList(lua_State *L);
    static int eventUnregister(lua_State *L);
    static int eventRaise(lua_State *L);
    static int eventProperty(lua_State *L);
    static int eventString(lua_State *L);
    static int eventXml(lua_State *L);

    static Group *groupNew(lua_State *L);
    static int groupGet(lua_State *L);
    static int groupDelete(lua_State *L);
    static int groupEnable(lua_State *L);
    static int groupProperty(lua_State *L);
    static int groupString(lua_State *L);
    static int groupXml(lua_State *L);
    static int groupAccelerators(lua_State *L);
    static int groupAliases(lua_State *L);
    static int groupGroups(lua_State *L);
    static int groupTimers(lua_State *L);
    static int groupTriggers(lua_State *L);
    static int groupVariables(lua_State *L);

    static Timer * timerNew(lua_State *L);
    static int timerGet(lua_State *L);
    static int timerDelete(lua_State *L);
    static int timerFire(lua_State *L);
    static int timerEnable(lua_State *L);
    static int timerProperty(lua_State *L);
    static int timerString(lua_State *L);
    static int timerXml(lua_State *L);

    static Trigger * triggerNew(lua_State *L);
    static int triggerGet(lua_State *L);
    static int triggerTest(lua_State *L);
    static int triggerDelete(lua_State *L);
    static int triggerFire(lua_State *L);
    static int triggerEnable(lua_State *L);
    static int triggerProperty(lua_State *L);
    static int triggerString(lua_State *L);
    static int triggerXml(lua_State *L);

    static Variable * variableNew(lua_State *L);
    static int variableGet(lua_State *L);
    static int variableDelete(lua_State *L);
    static int variableProperty(lua_State *L);
    static int variableString(lua_State *L);
    static int variableXml(lua_State *L);

    static int colorNote(lua_State *L);
    static int colorTell(lua_State *L);
    static int note(lua_State *L);
    static int tell(lua_State *L);
    static int print(lua_State *L);
    static int hyperlink(lua_State *L);

    static int openLog(lua_State *L);
    static int logNote(lua_State *L);
    static int closeLog(lua_State *L);
    static int isLogging(lua_State *L);
    static int pauseLog(lua_State *L);

    static int send(lua_State *L);
    static int sendAlias(lua_State *L);
    static int sendGmcp(lua_State *L);
    static int simulate(lua_State *L);

    static int setVariable(lua_State *L);
    static int getVariable(lua_State *L);
    static int deleteVariable(lua_State *L);

    static int jsonDecode(lua_State *L);
    static int jsonEncode(lua_State *L);

    static int isConnected(lua_State *L);
    static int connect(lua_State *L);
    static int disconnect(lua_State *L);

    static int playSound(lua_State *L);

private:
    static int callEventHandler(lua_State *L, Event *event, const QVariantList &args);
    static int callEventHandler(lua_State *L, Event *event, const QString &cmd, const QString &json);

    lua_State *m_global;
    QString m_chunk;
    QString m_script;
};

#endif // ENGINE_H
