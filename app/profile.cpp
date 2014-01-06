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


#include "profile.h"
#include "engine.h"
#include "mapengine.h"
#include "xmlexception.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>

Profile::Profile(QObject *parent) :
    QObject(parent)
{
    m_dirty = true;
    m_engine = 0;

    m_rootGroup = new Group(this);
    setActiveGroup(m_rootGroup);

    m_port = 23;
    m_autoConnect = false;
    m_autoReconnect = false;

    m_background = QColor(Qt::black);
    m_commandForeground = QColor("#AA5500");
    m_commandBackground = QColor(Qt::black);
    m_noteForeground = QColor(Qt::cyan);
    m_noteBackground = QColor(Qt::black);
    m_mapBackground = QColor(Qt::lightGray);

    m_autoWrap = false;
    m_wrapColumn = 81;

    m_autoLog = false;
    m_logDirectory = QDir::homePath() + "/Mudder/logs";
    m_logFilename = "%P_%D_%T.html";

    m_scriptPrefix = "/";
    m_commandSeparator = ';';
    m_clearCommandAfterSend = false;

    setOutputFont("Lucida Console", 10);
    setInputFont("Courier New", 8);
}

Profile::Profile(const Profile &rhs, QObject *parent) :
    QObject(parent)
{
    m_rootGroup = 0;
    m_activeGroup = 0;

    clone(rhs);
}

void Profile::copyPreferences(const Profile &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    m_name = rhs.m_name;
    m_filename = rhs.m_filename;
    m_scriptFilename = rhs.m_scriptFilename;

    m_address = rhs.m_address;
    m_port = rhs.m_port;
    m_autoConnect = rhs.m_autoConnect;
    m_autoReconnect = rhs.m_autoReconnect;

    m_background = rhs.m_background;
    m_commandBackground = rhs.m_commandBackground;
    m_commandForeground = rhs.m_commandForeground;
    m_noteBackground = rhs.m_noteBackground;
    m_noteForeground = rhs.m_noteForeground;

    m_autoWrap = rhs.m_autoWrap;
    m_wrapColumn = rhs.m_wrapColumn;

    m_outputFont = rhs.m_outputFont;
    m_inputFont = rhs.m_inputFont;
    m_outputAntiAlias = rhs.m_outputAntiAlias;
    m_inputAntiAlias = rhs.m_inputAntiAlias;

    m_autoLog = rhs.m_autoLog;
    m_logDirectory = rhs.m_logDirectory;
    m_logFilename = rhs.m_logFilename;

    m_scriptPrefix = rhs.m_scriptPrefix;
    m_commandSeparator = rhs.m_commandSeparator;
    m_clearCommandAfterSend = rhs.m_clearCommandAfterSend;

    setDirty(true);
}

void Profile::copySettings(const Profile &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    if (m_rootGroup)
    {
        delete m_rootGroup;
    }

    m_rootGroup = new Group(this);
    Q_CHECK_PTR(m_rootGroup);
    *m_rootGroup = *rhs.m_rootGroup;

    m_variables.clear();
    remapVariables(m_rootGroup);
    initTimers();

    m_activeGroup = m_rootGroup;

    setDirty(true);
}

void Profile::clone(const Profile &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    copyPreferences(rhs);
    copySettings(rhs);
}

void Profile::remapVariables(Group *group)
{
    foreach (Variable *variable, group->variables())
    {
        m_variables[variable->name()] = variable;
    }

    foreach (Group *child, group->groups())
    {
        remapVariables(child);
    }
}

Profile & Profile::operator =(const Profile &rhs)
{
    clone(rhs);
    return *this;
}

bool Profile::operator ==(const Profile &rhs)
{
    if (m_name != rhs.m_name)
    {
        return false;
    }
    if (m_filename != rhs.m_filename)
    {
        return false;
    }
    if (m_scriptFilename != rhs.m_scriptFilename)
    {
        return false;
    }

    if (m_address != rhs.m_address)
    {
        return false;
    }
    if (m_port != rhs.m_port)
    {
        return false;
    }
    if (m_autoConnect != rhs.m_autoConnect)
    {
        return false;
    }
    if (m_autoReconnect != rhs.m_autoReconnect)
    {
        return false;
    }

    if (m_background != rhs.m_background)
    {
        return false;
    }
    if (m_commandBackground != rhs.m_commandBackground)
    {
        return false;
    }
    if (m_commandForeground != rhs.m_commandForeground)
    {
        return false;
    }
    if (m_noteBackground != rhs.m_noteBackground)
    {
        return false;
    }
    if (m_noteForeground != rhs.m_noteForeground)
    {
        return false;
    }

    if (m_autoWrap != rhs.m_autoWrap)
    {
        return false;
    }
    if (m_wrapColumn != rhs.m_wrapColumn)
    {
        return false;
    }

    if (m_autoLog != rhs.m_autoLog)
    {
        return false;
    }
    if (m_logDirectory != rhs.m_logDirectory)
    {
        return false;
    }
    if (m_logFilename != rhs.m_logFilename)
    {
        return false;
    }

    if (m_scriptPrefix != rhs.m_scriptPrefix)
    {
        return false;
    }
    if (m_commandSeparator != rhs.m_commandSeparator)
    {
        return false;
    }
    if (m_clearCommandAfterSend != rhs.m_clearCommandAfterSend)
    {
        return false;
    }

    if (m_outputFont != rhs.m_outputFont)
    {
        return false;
    }
    if (m_inputFont != rhs.m_inputFont)
    {
        return false;
    }
    if (m_outputAntiAlias != rhs.m_outputAntiAlias)
    {
        return false;
    }
    if (m_inputAntiAlias != rhs.m_inputAntiAlias)
    {
        return false;
    }

    return true;
}

bool Profile::operator !=(const Profile &rhs)
{
    return !(*this == rhs);
}

void Profile::setDirty(bool flag)
{
    m_dirty = flag;

    emit changed();
}

void Profile::setOutputFont(const QFont &f, bool antiAlias)
{
    m_outputFont = f;
    m_outputFont.setStyleHint(QFont::TypeWriter);
    m_outputFont.setStyleStrategy(antiAlias?QFont::PreferAntialias:QFont::NoAntialias);
    m_outputAntiAlias = antiAlias;

    emit outputFontChanged();
}

void Profile::setOutputFont(const QString &name, int size, bool antiAlias)
{
    setOutputFont(QFont(name, size), antiAlias);
}

void Profile::setInputFont(const QFont &f, bool antiAlias)
{
    m_inputFont = f;
    m_inputFont.setStyleHint(QFont::TypeWriter);
    m_inputFont.setStyleStrategy(antiAlias?QFont::PreferAntialias:QFont::NoAntialias);
    m_inputAntiAlias = antiAlias;

    emit inputFontChanged();
}

void Profile::setInputFont(const QString &name, int size, bool antiAlias)
{
    setInputFont(QFont(name, size), antiAlias);
}

void Profile::uplevelGroup()
{
    Group *group = m_activeGroup->group();
    if (group == 0)
    {
        group = rootGroup();
    }
    setActiveGroup(group);
}

void Profile::initTimers(Group *group)
{
    if (group == 0)
    {
        group = rootGroup();
    }

    foreach (Group *child, group->groups())
    {
        initTimers(child);
    }

    foreach (Timer *timer, group->timers())
    {
        timer->setProfile(this);
    }
}

void Profile::addGroup(Group *group)
{
    group->setParent(activeGroup());
    activeGroup()->addGroup(group);
    setActiveGroup(group);

    setDirty(true);
}

void Profile::addAccelerator(Accelerator *accelerator)
{
    accelerator->setParent(activeGroup());
    activeGroup()->addAccelerator(accelerator);

    setDirty(true);
}

void Profile::addAlias(Alias *alias)
{
    alias->setParent(activeGroup());
    activeGroup()->addAlias(alias);

    setDirty(true);
}

void Profile::addTimer(Timer *timer)
{
    timer->setParent(activeGroup());
    timer->setProfile(this);
    activeGroup()->addTimer(timer);

    setDirty(true);
}

void Profile::addTrigger(Trigger *trigger)
{
    trigger->setParent(activeGroup());
    activeGroup()->addTrigger(trigger);

    setDirty(true);
}

void Profile::addVariable(Variable *variable)
{
    variable->setParent(activeGroup());
    activeGroup()->addVariable(variable);
    m_variables[variable->name()] = variable;

    setDirty(true);
}

bool Profile::deleteGroup(Group *group)
{
    Q_ASSERT(group != 0);
    if (group == 0)
    {
        return false;
    }

    group->enable(false);

    Group *parent = group->group();
    Q_ASSERT(parent != 0);
    if (parent == 0)
    {
        qDebug() << "deleteGroup, invalid parent";
        return false;
    }

    if (group == activeGroup())
    {
        setActiveGroup(parent);
    }

    bool result = parent->deleteGroup(group);

    delete group;

    setDirty(true);

    return result;
}

bool Profile::deleteAccelerator(Accelerator *accelerator)
{
    Q_ASSERT(accelerator != 0);
    if (accelerator == 0)
    {
        return false;
    }

    accelerator->enable(false);

    Group *parent = accelerator->group();
    Q_ASSERT(parent != 0);
    if (parent == 0)
    {
        qDebug() << "deleteAccelerator, invalid parent";
        return false;
    }

    bool result = parent->deleteAccelerator(accelerator);

    delete accelerator;

    setDirty(true);

    return result;
}

bool Profile::deleteAlias(Alias *alias)
{
    Q_ASSERT(alias != 0);
    if (alias == 0)
    {
        return false;
    }

    alias->enable(false);

    Group *parent = alias->group();
    Q_ASSERT(parent != 0);
    if (parent == 0)
    {
        qDebug() << "deleteAlias, invalid parent";
        return false;
    }

    bool result = parent->deleteAlias(alias);

    delete alias;

    return result;
}

bool Profile::deleteTimer(Timer *timer)
{
    Q_ASSERT(timer != 0);
    if (timer == 0)
    {
        return false;
    }

    timer->enable(false);

    Group *parent = timer->group();
    Q_ASSERT(parent != 0);
    if (parent == 0)
    {
        qDebug() << "deleteTimer, invalid parent";
        return false;
    }

    bool result = parent->deleteTimer(timer);

    delete timer;

    setDirty(true);

    return result;
}

bool Profile::deleteTrigger(Trigger *trigger)
{
    Q_ASSERT(trigger != 0);
    if (trigger == 0)
    {
        return false;
    }

    trigger->enable(false);

    Group *parent = trigger->group();
    Q_ASSERT(parent != 0);
    if (parent == 0)
    {
        qDebug() << "deleteTrigger, invalid parent";
        return false;
    }

    bool result = parent->deleteTrigger(trigger);

    delete trigger;

    setDirty(true);

    return result;
}

bool Profile::deleteVariable(Variable *variable)
{
    Q_ASSERT(variable != 0);
    if (variable == 0)
    {
        return false;
    }

    Group *parent = variable->group();
    Q_ASSERT(parent != 0);
    if (parent == 0)
    {
        qDebug() << "deleteVariable, invalid parent";
        return false;
    }

    bool result = parent->deleteVariable(variable);
    result = m_variables.remove(variable->name()) && result;

    delete variable;

    setDirty(true);

    return result;
}

Group * Profile::findGroup(const QString &name, Group *parent)
{
    QString cleanPath(QDir::cleanPath(name));
    Group *start = parent;
    if (start == 0)
    {
        if (cleanPath.startsWith('/'))
        {
            start = rootGroup();
            cleanPath.remove(0, 1);
        }
        else
        {
            start = activeGroup();
        }
    }
    Q_ASSERT(start != 0);

    if (cleanPath.isEmpty())
    {
        return start;
    }

    QStringList names(cleanPath.split('/', QString::SkipEmptyParts));
    Group *match = 0;
    foreach (QString name, names)
    {
        match = 0;
        foreach (Group *group, start->groups())
        {
            if (name.compare(group->name(), Qt::CaseInsensitive) == 0)
            {
                start = match = group;
                break;
            }
        }

        if (match == 0)
        {
            break;
        }
    }

    return match;
}

Accelerator * Profile::findAccelerator(const QString &name, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);

    QString realName(QDir::cleanPath(name));
    if (realName.isEmpty())
    {
        return 0;
    }

    if (realName.contains('/'))
    {
        QStringList names(realName.split('/'));
        realName = names.takeLast();

        parentGroup = findGroup(names.join('/'), parent);
        if (parentGroup == 0)
        {
            return 0;
        }
    }

    foreach (Accelerator *accelerator, parentGroup->accelerators())
    {
        if (accelerator->name() == realName)
        {
            return accelerator;
        }
    }

    if (!name.contains('/'))
    {
        foreach (Group *group, parentGroup->groups())
        {
            Accelerator *accelerator = findAccelerator(realName, group);
            if (accelerator != 0)
            {
                return accelerator;
            }
        }
    }

    return 0;
}

Alias * Profile::findAlias(const QString &name, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);

    QString realName(QDir::cleanPath(name));
    if (realName.isEmpty())
    {
        return 0;
    }

    if (realName.contains('/'))
    {
        QStringList names(realName.split('/'));
        realName = names.takeLast();

        parentGroup = findGroup(names.join('/'), parent);
        if (parentGroup == 0)
        {
            return 0;
        }
    }

    foreach (Alias *alias, parentGroup->aliases())
    {
        if (alias->name() == realName)
        {
            return alias;
        }
    }

    if (!name.contains('/'))
    {
        foreach (Group *group, parentGroup->groups())
        {
            Alias *alias = findAlias(realName, group);
            if (alias != 0)
            {
                return alias;
            }
        }
    }

    return 0;
}

Timer * Profile::findTimer(const QString &name, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);

    QString realName(QDir::cleanPath(name));
    if (realName.isEmpty())
    {
        return 0;
    }

    if (realName.contains('/'))
    {
        QStringList names(realName.split('/'));
        realName = names.takeLast();

        parentGroup = findGroup(names.join('/'), parent);
        if (parentGroup == 0)
        {
            return 0;
        }
    }

    foreach (Timer *timer, parentGroup->timers())
    {
        if (timer->name() == realName)
        {
            return timer;
        }
    }

    if (!name.contains('/'))
    {
        foreach (Group *group, parentGroup->groups())
        {
            Timer *timer = findTimer(realName, group);
            if (timer != 0)
            {
                return timer;
            }
        }
    }

    return 0;
}

Trigger * Profile::findTrigger(const QString &name, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);

    QString realName(QDir::cleanPath(name));
    if (realName.isEmpty())
    {
        return 0;
    }

    if (realName.contains('/'))
    {
        QStringList names(realName.split('/'));
        realName = names.takeLast();

        parentGroup = findGroup(names.join('/'), parent);
        if (parentGroup == 0)
        {
            return 0;
        }
    }

    foreach (Trigger *trigger, parentGroup->triggers())
    {
        if (trigger->name() == realName)
        {
            return trigger;
        }
    }

    if (!name.contains('/'))
    {
        foreach (Group *group, parentGroup->groups())
        {
            Trigger *trigger = findTrigger(realName, group);
            if (trigger != 0)
            {
                return trigger;
            }
        }
    }

    return 0;
}

Variable * Profile::findVariable(const QString &name)
{
    VariableMap::const_iterator it = m_variables.find(name);
    if (it == m_variables.end())
    {
        return 0;
    }

    return it.value();
}

bool Profile::existingGroup(Group *item, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);
    if (parentGroup == 0 || item == 0)
    {
        return false;
    }

    foreach (Group *group, parentGroup->groups())
    {
        if (existingGroup(item, group))
        {
            return true;
        }
    }

    return parentGroup->groups().indexOf(item) >= 0;
}

bool Profile::existingAccelerator(Accelerator *item, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);
    if (parentGroup == 0 || item == 0)
    {
        return false;
    }

    foreach (Group *group, parentGroup->groups())
    {
        if (existingAccelerator(item, group))
        {
            return true;
        }
    }

    return parentGroup->accelerators().indexOf(item) >= 0;
}

bool Profile::existingAlias(Alias *item, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);
    if (parentGroup == 0 || item == 0)
    {
        return false;
    }

    foreach (Group *group, parentGroup->groups())
    {
        if (existingAlias(item, group))
        {
            return true;
        }
    }

    return parentGroup->aliases().indexOf(item) >= 0;
}

bool Profile::existingTimer(Timer *item, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);
    if (parentGroup == 0 || item == 0)
    {
        return false;
    }

    foreach (Group *group, parentGroup->groups())
    {
        if (existingTimer(item, group))
        {
            return true;
        }
    }

    return parentGroup->timers().indexOf(item) >= 0;
}

bool Profile::existingTrigger(Trigger *item, Group *parent)
{
    Group *parentGroup = (parent == 0) ? activeGroup() : parent;
    Q_ASSERT(parentGroup != 0);
    if (parentGroup == 0 || item == 0)
    {
        return false;
    }

    foreach (Group *group, parentGroup->groups())
    {
        if (existingTrigger(item, group))
        {
            return true;
        }
    }

    return parentGroup->triggers().indexOf(item) >= 0;
}

bool Profile::existingVariable(Variable *item)
{
    Q_ASSERT(item != 0);
    if (item == 0)
    {
        return false;
    }

    return findVariable(item->name()) != 0;
}

bool Profile::enableGroup(const QString &name, bool flag)
{
    Group *group = findGroup(name);
    if (group == 0)
    {
        return false;
    }

    group->enable(flag);
    setDirty(true);
    return true;
}

bool Profile::enableAccelerator(const QString &name, bool flag)
{
    Accelerator *accelerator = findAccelerator(name, rootGroup());
    if (accelerator == 0)
    {
        return false;
    }

    accelerator->enable(flag);
    setDirty(true);
    return true;
}

bool Profile::enableAlias(const QString &name, bool flag)
{
    Alias *alias = findAlias(name, rootGroup());
    if (alias == 0)
    {
        return false;
    }

    alias->enable(flag);
    setDirty(true);
    return true;
}

bool Profile::enableTimer(const QString &name, bool flag)
{
    Timer *timer = findTimer(name, rootGroup());
    if (timer == 0)
    {
        return false;
    }

    timer->enable(flag);
    setDirty(true);
    return true;
}

bool Profile::enableTrigger(const QString &name, bool flag)
{
    Trigger *trigger = findTrigger(name, rootGroup());
    if (trigger == 0)
    {
        return false;
    }

    trigger->enable(flag);
    setDirty(true);
    return true;
}


void Profile::setVariable(const QString &name, const QVariant &value)
{
    Variable *variable = findVariable(name);
    if (variable)
    {
        if (value == variable->contents())
        {
            return;
        }

        variable->setContents(value);
        setDirty(true);
        return;
    }

    variable = new Variable(this);
    variable->setName(name);
    variable->setContents(value);
    addVariable(variable);
}

QVariant Profile::getVariable(const QString &name)
{
    Variable *variable = findVariable(name);
    if (variable)
    {
        return variable->contents();
    }

    return QVariant();
}

void Profile::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartDocument();
    xml.writeStartElement("mudder");

    xml.writeAttribute("version", QApplication::applicationVersion());
    xml.writeAttribute("saved", QDateTime::currentDateTime().toString());

    xml.writeStartElement("profile");
    xml.writeAttribute("name", name());

    xml.writeStartElement("host");
    xml.writeAttribute("connect", autoConnect()?"y":"n");
    xml.writeAttribute("reconnect", autoReconnect()?"y":"n");
    xml.writeAttribute("address", address());
    xml.writeAttribute("port", QString::number(port()));
    xml.writeEndElement();

    xml.writeStartElement("commandLine");
    xml.writeAttribute("clear", clearCommandAfterSend()?"y":"n");
    xml.writeAttribute("separator", commandSeparator());
    xml.writeEndElement();

    xml.writeStartElement("logging");
    xml.writeAttribute("auto", autoLog()?"y":"n");
    xml.writeTextElement("directory", logDirectory());
    xml.writeTextElement("filename", logFilename());
    xml.writeEndElement();

    xml.writeStartElement("scripting");
    xml.writeAttribute("file", scriptFilename());
    xml.writeAttribute("prefix", scriptPrefix());
    xml.writeEndElement();

    xml.writeStartElement("display");
    xml.writeAttribute("autowrap", autoWrap()?"y":"n");
    xml.writeAttribute("wrap", QString::number(wrapColumn()));

    xml.writeStartElement("outputFont");
    xml.writeAttribute("family", outputFont().family());
    xml.writeAttribute("size", QString::number(outputFont().pointSize()));
    xml.writeAttribute("antialias", isOutputAntiAliased()?"y":"n");
    xml.writeEndElement();

    xml.writeStartElement("inputFont");
    xml.writeAttribute("family", inputFont().family());
    xml.writeAttribute("size", QString::number(inputFont().pointSize()));
    xml.writeAttribute("antialias", isInputAntiAliased()?"y":"n");
    xml.writeEndElement();

    xml.writeStartElement("colors");
    xml.writeAttribute("bg", background().name());

    xml.writeStartElement("command");
    xml.writeAttribute("fg", commandForeground().name());
    xml.writeAttribute("bg", commandBackground().name());
    xml.writeEndElement();

    xml.writeStartElement("note");
    xml.writeAttribute("fg", noteForeground().name());
    xml.writeAttribute("bg", noteBackground().name());
    xml.writeEndElement();

    xml.writeEndElement(); // colors
    xml.writeEndElement(); // display

    xml.writeStartElement("map");
    xml.writeAttribute("bg", mapBackground().name());
    xml.writeAttribute("file", mapFilename());
    xml.writeEndElement();

    xml.writeEndElement(); // profile

    m_rootGroup->toXml(xml, "settings");

    xml.writeEndDocument();
}

void Profile::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isStartElement())
        {
            if (xml.name() == "profile")
            {
                try
                {
                    readProfile(xml);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
            else if (xml.name() == "settings")
            {
                try
                {
                    m_rootGroup->fromXml(xml, "settings");
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
        }
    }

    m_variables.clear();
    remapVariables(m_rootGroup);
    initTimers();

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

void Profile::readProfile(QXmlStreamReader &xml)
{
    QStringList warnings;

    setName(xml.attributes().value("name").toString());

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "profile")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "host")
            {
                setAutoConnect(xml.attributes().value("connect").compare("y", Qt::CaseInsensitive) == 0);
                setAutoReconnect(xml.attributes().value("reconnect").compare("y", Qt::CaseInsensitive) == 0);

                QString address(xml.attributes().value("address").toString());
                if (address.isEmpty())
                {
                    warnings.append(tr("XML: Line %1; missing or empty host address").arg(xml.lineNumber()));
                }
                setAddress(address);

                bool valid = true;
                int port = xml.attributes().value("port").toString().toInt(&valid);
                if (!valid)
                {
                    port = 23;
                    warnings.append(tr("XML: Line %1; missing or invalid host port, defaulting to 23").arg(xml.lineNumber()));
                }
                setPort(port);
            }
            else if (xml.name() == "commandLine")
            {
                QString sep(xml.attributes().value("separator").toString());
                if (!sep.isEmpty())
                {
                    if (sep.length() > 1)
                    {
                        warnings.append(tr("XML: Line %1; command separator should be a single character").arg(xml.lineNumber()));
                    }
                    setCommandSeparator(sep.at(0));
                }

                setClearCommandAfterSend(xml.attributes().value("clear").compare("n", Qt::CaseInsensitive) != 0);
            }
            else if (xml.name() == "logging")
            {
                try
                {
                    readLogging(xml);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
            else if (xml.name() == "scripting")
            {
                QString prefix(xml.attributes().value("prefix").toString());
                if (!prefix.isEmpty())
                {
                   setScriptPrefix(prefix);
                }
                QString script(xml.attributes().value("file").toString());
                if (!script.isEmpty())
                {
                    setScriptFilename(script);
                }
            }
            else if (xml.name() == "display")
            {
                try
                {
                    readDisplay(xml);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
            else if (xml.name() == "mapper")
            {
                QString colorString(xml.attributes().value("bg").toString());
                QColor bg(colorString);
                if (bg.isValid())
                {
                    setMapBackground(bg);
                }
                else if (!colorString.isEmpty())
                {
                    warnings.append(tr("XML: Line %1; invalid mapper background color (%2)").arg(xml.lineNumber()).arg(colorString));
                }

                QString filename(xml.attributes().value("file").toString());
                if (!filename.isEmpty())
                {
                    setMapFilename(filename);
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

void Profile::readDisplay(QXmlStreamReader &xml)
{
    QStringList warnings;

    setAutoWrap(xml.attributes().value("autowrap").compare("y", Qt::CaseInsensitive) == 0);

    bool valid = true;
    int wrap = xml.attributes().value("wrap").toString().toInt(&valid);
    if (!valid)
    {
        wrap = 81;
    }
    setWrapColumn(qBound(10, wrap, 250));


    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "display")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "outputFont")
            {
                QString family(xml.attributes().value("family").toString());
                if (family.isEmpty())
                {
                    family = "Lucida Console";
                }

                bool valid = true;
                int size = xml.attributes().value("size").toString().toInt(&valid);
                if (!valid)
                {
                    size = 10;
                }
                size = qBound(6, size, 30);

                bool antiAlias = true;
                if (!xml.attributes().value("antialias").isEmpty())
                {
                    antiAlias = xml.attributes().value("antialias").compare("n", Qt::CaseInsensitive) != 0;
                }
                setOutputFont(family, size, antiAlias);
            }
            else if (xml.name() == "inputFont")
            {
                QString family(xml.attributes().value("family").toString());
                if (family.isEmpty())
                {
                    family = "Courier New";
                }

                bool valid = true;
                int size = xml.attributes().value("size").toString().toInt(&valid);
                if (!valid)
                {
                    size = 8;
                }
                size = qBound(6, size, 20);

                bool antiAlias = true;
                if (!xml.attributes().value("antialias").isEmpty())
                {
                    antiAlias = xml.attributes().value("antialias").compare("n", Qt::CaseInsensitive) != 0;
                }
                setInputFont(family, size, antiAlias);
            }
            else if (xml.name() == "colors")
            {
                try
                {
                    readColors(xml);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

void Profile::readColors(QXmlStreamReader &xml)
{
    QStringList warnings;

    QString colorString(xml.attributes().value("bg").toString());
    QColor bg(colorString);
    if (bg.isValid())
    {
        setBackground(bg);
    }
    else if (!colorString.isEmpty())
    {
        warnings.append(tr("XML: Line %1; invalid console background color (%2)").arg(xml.lineNumber()).arg(colorString));
    }

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "colors")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "command")
            {
                colorString = xml.attributes().value("fg").toString();
                QColor fg(colorString);
                if (fg.isValid())
                {
                    setCommandForeground(fg);
                }
                else if (!colorString.isEmpty())
                {
                    warnings.append(tr("XML: Line %1; invalid command line foreground color (%2)").arg(xml.lineNumber()).arg(colorString));
                }

                colorString = xml.attributes().value("bg").toString();
                QColor bg(colorString);
                if (bg.isValid())
                {
                    setCommandBackground(bg);
                }
                else if (!colorString.isEmpty())
                {
                    warnings.append(tr("XML: Line %1; invalid command line background color (%2)").arg(xml.lineNumber()).arg(colorString));
                }
            }
            else if (xml.name() == "note")
            {
                colorString = xml.attributes().value("fg").toString();
                QColor fg(colorString);
                if (fg.isValid())
                {
                    setNoteForeground(fg);
                }
                else if (!colorString.isEmpty())
                {
                    warnings.append(tr("XML: Line %1; invalid note foreground color (%2)").arg(xml.lineNumber()).arg(colorString));
                }

                colorString = xml.attributes().value("bg").toString();
                QColor bg(colorString);
                if (bg.isValid())
                {
                    setNoteBackground(bg);
                }
                else if (!colorString.isEmpty())
                {
                    warnings.append(tr("XML: Line %1; invalid note background color (%2)").arg(xml.lineNumber()).arg(colorString));
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

void Profile::readLogging(QXmlStreamReader &xml)
{
    setAutoLog(xml.attributes().value("auto").compare("y", Qt::CaseInsensitive) == 0);

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "logging")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "directory")
            {
                setLogDirectory(QDir::cleanPath(xml.readElementText()));
            }
            else if (xml.name() == "filename")
            {
                setLogFilename(QFileInfo(xml.readElementText()).fileName());
            }
        }
    }
}

void Profile::appendXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isStartElement())
        {
            if (xml.name() == "accelerator")
            {
                Accelerator *accelerator = 0;
                try
                {
                    accelerator = new Accelerator(this);
                    Q_CHECK_PTR(accelerator);
                    accelerator->fromXml(xml);

                    Accelerator *lookup = findAccelerator(accelerator->name(), rootGroup());
                    if (lookup)
                    {
                        *lookup = *accelerator;
                        delete accelerator;
                    }
                    else
                    {
                        addAccelerator(accelerator);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete accelerator;
                }
            }
            else if (xml.name() == "alias")
            {
                Alias *alias = 0;
                try
                {
                    alias = new Alias(this);
                    Q_CHECK_PTR(alias);
                    alias->fromXml(xml);

                    Alias *lookup = findAlias(alias->name(), rootGroup());
                    if (lookup)
                    {
                        *lookup = *alias;
                        delete alias;
                    }
                    else
                    {
                        addAlias(alias);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete alias;
                }
            }
            else if (xml.name() == "group")
            {
                Group *group = 0;
                try
                {
                    group = new Group(this);
                    Q_CHECK_PTR(group);
                    group->fromXml(xml);

                    warnings.append(group->warnings());

                    Group *lookup = findGroup(group->name(), activeGroup());
                    if (lookup)
                    {
                        lookup->merge(*group);
                        delete group;
                    }
                    else
                    {
                        addGroup(group);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete group;
                }
            }
            else if (xml.name() == "timer")
            {
                Timer *timer = 0;
                try
                {
                    timer = new Timer(this);
                    Q_CHECK_PTR(timer);
                    timer->fromXml(xml);

                    Timer *lookup = findTimer(timer->name(), rootGroup());
                    if (lookup)
                    {
                        *lookup = *timer;
                        delete timer;
                    }
                    else
                    {
                        addTimer(timer);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete timer;
                }
            }
            else if (xml.name() == "trigger")
            {
                Trigger *trigger = 0;
                try
                {
                    trigger = new Trigger(this);
                    Q_CHECK_PTR(trigger);
                    trigger->fromXml(xml);

                    Trigger *lookup = findTrigger(trigger->name(), rootGroup());
                    if (lookup)
                    {
                        *lookup = *trigger;
                        delete trigger;
                    }
                    else
                    {
                        addTrigger(trigger);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete trigger;
                }
            }
            else if (xml.name() == "variable")
            {
                Variable *variable = 0;
                try
                {
                    variable = new Variable(this);
                    Q_CHECK_PTR(variable);
                    variable->fromXml(xml);

                    Variable *lookup = findVariable(variable->name());
                    if (lookup)
                    {
                        *lookup = *variable;
                        delete variable;
                    }
                    else
                    {
                        addVariable(variable);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete variable;
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
