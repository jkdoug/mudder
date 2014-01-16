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


#include "group.h"
#include "accelerator.h"
#include "alias.h"
#include "timer.h"
#include "trigger.h"
#include "variable.h"
#include "xmlexception.h"
#include <QDebug>

Group::Group(QObject *parent) :
    ProfileItem(parent)
{
    m_sortAccelerators = false;
    m_sortAliases = false;
    m_sortTimers = false;
    m_sortTriggers = false;
    m_sortVariables = false;
}

Group::Group(const Group &rhs, QObject *parent) :
    ProfileItem(parent)
{
    clone(rhs);
}

Group & Group::operator =(const Group &rhs)
{
    m_groups.clear();
    m_accelerators.clear();
    m_aliases.clear();
    m_timers.clear();
    m_triggers.clear();
    m_variables.clear();

    clone(rhs);
    return *this;
}

bool Group::operator <(const Group &rhs)
{
    return name().compare(rhs.name()) < 0;
}

void Group::merge(const Group &rhs)
{
    foreach (Group *group, rhs.m_groups)
    {
        bool found = false;
        foreach (Group *lookup, m_groups)
        {
            if (lookup->name() == group->name())
            {
                lookup->merge(*group);
                found = true;
                break;
            }
        }

        if (!found)
        {
            Group *copy = new Group(*group, this);
            Q_CHECK_PTR(copy);
            addGroup(copy);
        }
    }

    foreach (Accelerator *accelerator, rhs.m_accelerators)
    {
        bool found = false;
        foreach (Accelerator *lookup, m_accelerators)
        {
            if (lookup->name() == accelerator->name())
            {
                m_sortAccelerators = m_sortAccelerators || lookup->sequence() != accelerator->sequence();
                *lookup = *accelerator;
                found = true;
                break;
            }
        }

        if (!found)
        {
            Accelerator *copy = new Accelerator(*accelerator, this);
            Q_CHECK_PTR(copy);
            addAccelerator(copy);
        }
    }

    foreach (Alias *alias, rhs.m_aliases)
    {
        bool found = false;
        foreach (Alias *lookup, m_aliases)
        {
            if (lookup->name() == alias->name())
            {
                m_sortAliases = m_sortAliases || lookup->sequence() != alias->sequence();
                *lookup = *alias;
                found = true;
                break;
            }
        }

        if (!found)
        {
            Alias *copy = new Alias(*alias, this);
            Q_CHECK_PTR(copy);
            addAlias(copy);
        }
    }

    foreach (Timer *timer, rhs.m_timers)
    {
        bool found = false;
        foreach (Timer *lookup, m_timers)
        {
            if (lookup->name() == timer->name())
            {
                m_sortTimers = m_sortTimers || lookup->sequence() != timer->sequence();
                *lookup = *timer;
                found = true;
                break;
            }
        }

        if (!found)
        {
            Timer *copy = new Timer(*timer, this);
            Q_CHECK_PTR(copy);
            addTimer(copy);
        }
    }

    foreach (Trigger *trigger, rhs.m_triggers)
    {
        bool found = false;
        foreach (Trigger *lookup, m_triggers)
        {
            if (lookup->name() == trigger->name())
            {
                m_sortTriggers = m_sortTriggers || lookup->sequence() != trigger->sequence();
                *lookup = *trigger;
                found = true;
                break;
            }
        }

        if (!found)
        {
            Trigger *copy = new Trigger(*trigger, this);
            Q_CHECK_PTR(copy);
            addTrigger(copy);
        }
    }

    foreach (Variable *variable, rhs.m_variables)
    {
        bool found = false;
        foreach (Variable *lookup, m_variables)
        {
            if (lookup->name() == variable->name())
            {
                m_sortVariables = m_sortVariables || lookup->name() != variable->name();
                *lookup = *variable;
                found = true;
                break;
            }
        }

        if (!found)
        {
            Variable *copy = new Variable(*variable, this);
            Q_CHECK_PTR(copy);
            addVariable(copy);
        }
    }
}

void Group::addGroup(Group *group)
{
    m_sortAccelerators = true;
    m_sortAliases = true;
    m_sortTimers = true;
    m_sortTriggers = true;
    m_sortVariables = true;

    m_groups.append(group);
}

void Group::addAccelerator(Accelerator *accelerator)
{
    m_sortAccelerators = true;

    m_accelerators.append(accelerator);
}

void Group::addAlias(Alias *alias)
{
    m_sortAliases = true;

    m_aliases.append(alias);
}

void Group::addTimer(Timer *timer)
{
    m_sortTimers = true;

    m_timers.append(timer);
}

void Group::addTrigger(Trigger *trigger)
{
    m_sortTriggers = true;

    m_triggers.append(trigger);
}

void Group::addVariable(Variable *variable)
{
    m_sortVariables = true;

    m_variables.append(variable);
}

bool Group::deleteGroup(Group *group)
{
    m_sortAccelerators = true;
    m_sortAliases = true;
    m_sortTimers = true;
    m_sortTriggers = true;
    m_sortVariables = true;

    return m_groups.removeOne(group);
}

bool Group::deleteAccelerator(Accelerator *accelerator)
{
    m_sortAccelerators = true;

    return m_accelerators.removeOne(accelerator);
}

bool Group::deleteAlias(Alias *alias)
{
    m_sortAliases = true;

    return m_aliases.removeOne(alias);
}

bool Group::deleteTimer(Timer *timer)
{
    m_sortTimers = true;

    return m_timers.removeOne(timer);
}

bool Group::deleteTrigger(Trigger *trigger)
{
    m_sortTriggers = true;

    return m_triggers.removeOne(trigger);
}

bool Group::deleteVariable(Variable *variable)
{
    m_sortVariables = true;

    return m_variables.removeOne(variable);
}

const QStringList & Group::warnings()
{
    foreach (Group *group, groups())
    {
        m_warnings.append(group->warnings());
    }

    return m_warnings;
}

QString Group::path() const
{
    Group * up = qobject_cast<Group *>(parent());
    if (up == 0)
    {
        return "";
    }

    return QString("%1/%2").arg(up->path()).arg(name());
}

const QList<Accelerator *> & Group::sortedAccelerators()
{
    if (m_sortAccelerators)
    {
        m_sortedAccelerators = accelerators();
        foreach (Group *group, groups())
        {
            m_sortedAccelerators.append(group->sortedAccelerators());
        }
        m_sortAccelerators = false;
    }

    return m_sortedAccelerators;
}

const QList<Alias *> & Group::sortedAliases()
{
    if (m_sortAliases)
    {
        m_sortedAliases = aliases();
        foreach (Group *group, groups())
        {
            m_sortedAliases.append(group->sortedAliases());
        }

        qSort(m_sortedAliases.begin(), m_sortedAliases.end(), Alias::lessSequence);

        m_sortAliases = false;
    }

    return m_sortedAliases;
}

const QList<Timer *> & Group::sortedTimers()
{
    if (m_sortTimers)
    {
        m_sortedTimers = timers();
        foreach (Group *group, groups())
        {
            m_sortedTimers.append(group->sortedTimers());
        }

        qSort(m_sortedTimers.begin(), m_sortedTimers.end(), Timer::lessInterval);

        m_sortTimers = false;
    }

    return m_sortedTimers;
}

const QList<Trigger *> & Group::sortedTriggers()
{
    if (m_sortTriggers)
    {
        m_sortedTriggers = triggers();
        foreach (Group *group, groups())
        {
            m_sortedTriggers.append(group->sortedTriggers());
        }

        qSort(m_sortedTriggers.begin(), m_sortedTriggers.end(), Trigger::lessSequence);

        m_sortTriggers = false;
    }

    return m_sortedTriggers;
}

const QList<Variable *> & Group::sortedVariables()
{
    if (m_sortVariables)
    {
        m_sortedVariables = variables();
        foreach (Group *group, groups())
        {
            m_sortedVariables.append(group->sortedVariables());
        }

        qSort(m_sortedVariables.begin(), m_sortedVariables.end(), Variable::lessName);

        m_sortVariables = false;
    }

    return m_sortedVariables;
}

void Group::clone(const Group &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    ProfileItem::clone(rhs);

    m_sortAccelerators = false;
    m_sortAliases = false;
    m_sortTimers = false;
    m_sortTriggers = false;
    m_sortVariables = false;

    foreach (Group *group, rhs.m_groups)
    {
        Group *copy = new Group(*group, this);
        Q_CHECK_PTR(copy);
        addGroup(copy);
    }

    foreach (Accelerator *accelerator, rhs.m_accelerators)
    {
        Accelerator *copy = new Accelerator(*accelerator, this);
        Q_CHECK_PTR(copy);
        addAccelerator(copy);
    }

    foreach (Alias *alias, rhs.m_aliases)
    {
        Alias *copy = new Alias(*alias, this);
        Q_CHECK_PTR(copy);
        addAlias(copy);
    }

    foreach (Timer *timer, rhs.m_timers)
    {
        Timer *copy = new Timer(*timer, this);
        Q_CHECK_PTR(copy);
        addTimer(copy);
    }

    foreach (Trigger *trigger, rhs.m_triggers)
    {
        Trigger *copy = new Trigger(*trigger, this);
        Q_CHECK_PTR(copy);
        addTrigger(copy);
    }

    foreach (Variable *variable, rhs.m_variables)
    {
        Variable *copy = new Variable(*variable, this);
        Q_CHECK_PTR(copy);
        addVariable(copy);
    }
}

void Group::toXml(QXmlStreamWriter &xml, const QString &element)
{
    xml.writeStartElement(element);

    if (element == "group")
    {
        ProfileItem::toXml(xml);
    }

    foreach (Group *child, m_groups)
    {
        child->toXml(xml);
    }

    foreach (Accelerator *accelerator, m_accelerators)
    {
        accelerator->toXml(xml);
    }

    foreach (Alias *alias, m_aliases)
    {
        alias->toXml(xml);
    }

    foreach (Timer *timer, m_timers)
    {
        timer->toXml(xml);
    }

    foreach (Trigger *trigger, m_triggers)
    {
        trigger->toXml(xml);
    }

    foreach (Variable *variable, m_variables)
    {
        variable->toXml(xml);
    }

    xml.writeEndElement();
}

void Group::toXml(QXmlStreamWriter &xml)
{
    toXml(xml, "group");
}

void Group::fromXml(QXmlStreamReader &xml)
{
    ProfileItem::fromXml(xml);
}
