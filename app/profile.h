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


#ifndef PROFILE_H
#define PROFILE_H

#include "accelerator.h"
#include "alias.h"
#include "group.h"
#include "timer.h"
#include "trigger.h"
#include "variable.h"
#include <QChar>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Engine;
class MapEngine;

class Profile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString filename READ filename WRITE setFilename)
    Q_PROPERTY(QString scriptFilename READ scriptFilename WRITE setScriptFilename)
    Q_PROPERTY(bool dirty READ isDirty WRITE setDirty)

    Q_PROPERTY(QString address READ address WRITE setAddress)
    Q_PROPERTY(int port READ port WRITE setPort)
    Q_PROPERTY(bool autoConnect READ autoConnect WRITE setAutoConnect)
    Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect)

    Q_PROPERTY(QColor background READ background WRITE setBackground)
    Q_PROPERTY(QColor commandBackground READ commandBackground WRITE setCommandBackground)
    Q_PROPERTY(QColor commandForeground READ commandForeground WRITE setCommandForeground)
    Q_PROPERTY(QColor noteBackground READ noteBackground WRITE setNoteBackground)
    Q_PROPERTY(QColor noteForeground READ noteForeground WRITE setNoteForeground)

    Q_PROPERTY(QFont outputFont READ outputFont WRITE setOutputFont)
    Q_PROPERTY(bool outputAntiAliased READ isOutputAntiAliased)
    Q_PROPERTY(QFont inputFont READ inputFont WRITE setInputFont)
    Q_PROPERTY(bool inputAntiAliased READ isInputAntiAliased)

    Q_PROPERTY(bool autoWrap READ autoWrap WRITE setAutoWrap)
    Q_PROPERTY(int wrapColumn READ wrapColumn WRITE setWrapColumn)

    Q_PROPERTY(bool autoLog READ autoLog WRITE setAutoLog)
    Q_PROPERTY(QString logDirectory READ logDirectory WRITE setLogDirectory)
    Q_PROPERTY(QString logFilename READ logFilename WRITE setLogFilename)

    Q_PROPERTY(QString scriptPrefix READ scriptPrefix WRITE setScriptPrefix)
    Q_PROPERTY(QChar commandSeparator READ commandSeparator WRITE setCommandSeparator)
    Q_PROPERTY(bool clearCommandAfterSend READ clearCommandAfterSend WRITE setClearCommandAfterSend)

    Q_PROPERTY(QString activeGroupPath READ activeGroupPath)

public:
    explicit Profile(QObject *parent = 0);
    Profile(const Profile &rhs, QObject *parent = 0);

    void copyPreferences(const Profile &rhs);
    void copySettings(const Profile &rhs);

    Profile & operator =(const Profile &rhs);
    bool operator ==(const Profile &rhs);
    bool operator !=(const Profile &rhs);

    const QString & name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    const QString & filename() const { return m_filename; }
    void setFilename(const QString &filename) { m_filename = filename; }
    const QString & scriptFilename() const { return m_scriptFilename; }
    void setScriptFilename(const QString &filename) { m_scriptFilename = filename; }
    const QString & mapFilename() const { return m_mapFilename; }
    void setMapFilename(const QString &filename) { m_mapFilename = filename; }
    bool isDirty() const { return m_dirty; }
    void setDirty(bool flag = true);
    Engine *engine() const { return m_engine; }
    void setEngine(Engine *engine) { m_engine = engine; }

    const QString & address() const { return m_address; }
    void setAddress(const QString &addr) { m_address = addr; }
    int port() const { return m_port; }
    void setPort(int port) { m_port = port; }
    bool autoConnect() const { return m_autoConnect; }
    void setAutoConnect(bool flag = true) { m_autoConnect = flag; }
    bool autoReconnect() const { return m_autoReconnect; }
    void setAutoReconnect(bool flag = true) { m_autoReconnect = flag; }

    const QColor & background() const { return m_background; }
    void setBackground(const QColor &color) { m_background = color; emit backgroundChanged(); }
    const QColor & commandBackground() const { return m_commandBackground; }
    void setCommandBackground(const QColor &color) { m_commandBackground = color; }
    const QColor & commandForeground() const { return m_commandForeground; }
    void setCommandForeground(const QColor &color) { m_commandForeground = color; }
    const QColor & noteBackground() const { return m_noteBackground; }
    void setNoteBackground(const QColor &color) { m_noteBackground = color; }
    const QColor & noteForeground() const { return m_noteForeground; }
    void setNoteForeground(const QColor &color) { m_noteForeground = color; }
    const QColor & mapBackground() const { return m_mapBackground; }
    void setMapBackground(const QColor &color) { m_mapBackground = color; emit mapBackgroundChanged(); }

    const QFont & outputFont() const { return m_outputFont; }
    void setOutputFont(const QFont &f, bool antiAlias = true);
    void setOutputFont(const QString &name, int size, bool antiAlias = true);
    bool isOutputAntiAliased() const { return m_outputAntiAlias; }
    const QFont & inputFont() const { return m_inputFont; }
    void setInputFont(const QFont &f, bool antiAlias = true);
    void setInputFont(const QString &name, int size, bool antiAlias = true);
    bool isInputAntiAliased() const { return m_inputAntiAlias; }

    bool autoWrap() const { return m_autoWrap; }
    void setAutoWrap(bool flag = true) { m_autoWrap = flag; }
    int wrapColumn() const { return m_wrapColumn; }
    void setWrapColumn(int column) { m_wrapColumn = column; }

    bool autoLog() const { return m_autoLog; }
    void setAutoLog(bool flag = true) { m_autoLog = flag; }
    const QString & logDirectory() const { return m_logDirectory; }
    void setLogDirectory(const QString &dir) { m_logDirectory = dir; }
    const QString & logFilename() const { return m_logFilename; }
    void setLogFilename(const QString &filename) { m_logFilename = filename; }

    const QString & scriptPrefix() const { return m_scriptPrefix; }
    void setScriptPrefix(const QString &prefix) { m_scriptPrefix = prefix; }
    const QChar & commandSeparator() const { return m_commandSeparator; }
    void setCommandSeparator(const QChar &separator) { m_commandSeparator = separator; }
    bool clearCommandAfterSend() const { return m_clearCommandAfterSend; }
    void setClearCommandAfterSend(bool flag = true) { m_clearCommandAfterSend = flag; }

    Group *rootGroup() const { return m_rootGroup; }
    Group *activeGroup() const { return m_activeGroup; }
    QString activeGroupPath() const { return activeGroup()->path(); }
    void uplevelGroup();
    void setActiveGroup(Group *group) { m_activeGroup = group; }

    void addGroup(Group *group);
    void addAccelerator(Accelerator *accelerator);
    void addAlias(Alias *alias);
    void addTimer(Timer *timer);
    void addTrigger(Trigger *trigger);
    void addVariable(Variable *variable, Group *parent = 0);

    bool deleteGroup(Group *group);
    bool deleteAccelerator(Accelerator *accelerator);
    bool deleteAlias(Alias *alias);
    bool deleteTimer(Timer *timer);
    bool deleteTrigger(Trigger *trigger);
    bool deleteVariable(Variable *variable);

    Group *findGroup(const QString &name, Group *parent = 0);
    Group *findParentGroup(const QString &name, Group *parent = 0);
    Accelerator *findAccelerator(const QString &name, Group *parent = 0);
    Alias *findAlias(const QString &name, Group *parent = 0);
    Timer *findTimer(const QString &name, Group *parent = 0);
    Trigger *findTrigger(const QString &name, Group *parent = 0);
    Variable *findVariable(const QString &name, Group *parent = 0);

    bool existingGroup(Group *item, Group *parent = 0);
    bool existingAccelerator(Accelerator *item, Group *parent = 0);
    bool existingAlias(Alias *item, Group *parent = 0);
    bool existingTimer(Timer *item, Group *parent = 0);
    bool existingTrigger(Trigger *item, Group *parent = 0);
    bool existingVariable(Variable *item);

    bool enableGroup(const QString &name, bool flag = true);
    bool enableAccelerator(const QString &name, bool flag = true);
    bool enableAlias(const QString &name, bool flag = true);
    bool enableTimer(const QString &name, bool flag = true);
    bool enableTrigger(const QString &name, bool flag = true);

    void setVariable(const QString &name, const QVariant &value);
    QVariant getVariable(const QString &name);

    void initTimers(Group *group = 0);

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);
    virtual void appendXml(QXmlStreamReader &xml);

signals:
    void changed();

    void backgroundChanged();
    void mapBackgroundChanged();
    void inputFontChanged();
    void outputFontChanged();

private:
    void clone(const Profile &rhs);

    void readProfile(QXmlStreamReader &xml);
    void readDisplay(QXmlStreamReader &xml);
    void readColors(QXmlStreamReader &xml);
    void readLogging(QXmlStreamReader &xml);

    QString m_name;
    QString m_filename;
    QString m_scriptFilename;
    QString m_mapFilename;
    bool m_dirty;
    Engine *m_engine;

    QString m_address;
    int m_port;
    bool m_autoConnect;
    bool m_autoReconnect;

    QColor m_background;
    QColor m_commandBackground;
    QColor m_commandForeground;
    QColor m_noteBackground;
    QColor m_noteForeground;
    QColor m_mapBackground;

    QFont m_outputFont;
    QFont m_inputFont;
    bool m_outputAntiAlias;
    bool m_inputAntiAlias;

    bool m_autoWrap;
    int m_wrapColumn;

    bool m_autoLog;
    QString m_logDirectory;
    QString m_logFilename;

    QString m_scriptPrefix;
    QChar m_commandSeparator;
    bool m_clearCommandAfterSend;

    Group *m_rootGroup;
    Group *m_activeGroup;

    VariableMap m_variables;
};

#endif // PROFILE_H
