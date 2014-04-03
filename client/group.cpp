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


#include "group.h"
#include "logger.h"
#include "xmlerror.h"
#include "accelerator.h"
#include "alias.h"
#include "event.h"
#include "timer.h"
#include "trigger.h"
#include "variable.h"
#include <QRegularExpression>

Group::Group(QObject *parent) :
    ProfileItem(parent)
{
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

QList<Accelerator *> Group::sortedAccelerators(bool all) const
{
    return sortedItems<Accelerator>(all);
}

QList<Alias *> Group::sortedAliases(bool all) const
{
    return sortedItems<Alias>(all);
}

QList<Event *> Group::sortedEvents(bool all) const
{
    return sortedItems<Event>(all);
}

QList<Group *> Group::sortedGroups(bool all) const
{
    return sortedItems<Group>(all);
}

QList<Timer *> Group::sortedTimers(bool all) const
{
    return sortedItems<Timer>(all);
}

QList<Trigger *> Group::sortedTriggers(bool all) const
{
    return sortedItems<Trigger>(all);
}

QList<Variable *> Group::sortedVariables(bool all) const
{
    return sortedItems<Variable>(all);
}

void Group::addItem(ProfileItem *item)
{
    LOG_TRACE("Group::addItem", path(), item->name());

    item->setParent(this);
    m_items << item;

    Group *group = qobject_cast<Group *>(item);
    if (group)
    {
        m_groups << group;
    }
}

bool Group::removeItem(ProfileItem *item)
{
    LOG_TRACE("Group::removeItem", path(), item->name());

    // TODO: remove groups, triggers, etc

    return m_items.removeOne(item);
}


void Group::toXml(QXmlStreamWriter &xml)
{
    LOG_TRACE("Group::toXml", path());

    if (!name().isEmpty())
    {
        ProfileItem::toXml(xml);
    }

    foreach (ProfileItem *item, m_items)
    {
        xml.writeStartElement(item->tagName());
        item->toXml(xml);
        xml.writeEndElement();
    }
}

void Group::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    QString name(xml.attributes().value("name").toString());
    if (!validateName(name, group() != 0))
    {
        XmlError *err = new XmlError(xml.lineNumber(), xml.columnNumber());
        if (name.isEmpty())
        {
            err->setMessage(tr("missing 'name' attribute"));
        }
        else
        {
            err->setMessage(tr("invalid value for 'name' attribute: %1").arg(name));
        }
        errors << err;

        static int unnamed = 0;
        name = QString("!unnamed%1").arg(++unnamed);
    }
    setName(name);

    enable(xml.attributes().value("enabled").compare("n", Qt::CaseInsensitive) != 0);

    LOG_TRACE("Group::fromXml", path());
}

template<class C>
QList<C *> Group::sortedItems(bool all) const
{
    QList<C *> list;
    foreach (ProfileItem *item, m_items)
    {
        C *sub = qobject_cast<C *>(item);
        if (sub)
        {
            list << sub;
        }
    }

    if (all)
    {
        foreach (Group *group, m_groups)
        {
            list << group->sortedItems<C>(all);
        }
    }

    qSort(list.begin(), list.end());

    return list;
}
