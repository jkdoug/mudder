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


#ifndef GROUP_H
#define GROUP_H

#include "profileitem.h"
#include <QList>

class Accelerator;
class Alias;
class Event;
class Timer;
class Trigger;
class Variable;
class XmlError;

class Group : public ProfileItem
{
    Q_OBJECT

    friend class Profile;

public:
    explicit Group(QObject *parent = 0);

    virtual int sequence() const { return 0; }

    int itemCount() const { return m_items.count(); }
    int itemIndex(ProfileItem *item) { return m_items.indexOf(item); }
    ProfileItem * item(int index) { return m_items.at(index); }

    QList<Accelerator *> sortedAccelerators(bool enabled = true, bool all = true) const;
    QList<Alias *> sortedAliases(bool enabled = true, bool all = true) const;
    QList<Event *> sortedEvents(bool enabled = true, bool all = true) const;
    QList<Group *> sortedGroups(bool enabled = true, bool all = true) const;
    QList<Timer *> sortedTimers(bool enabled = true, bool all = true) const;
    QList<Trigger *> sortedTriggers(bool enabled = true, bool all = true) const;
    QList<Variable *> sortedVariables(bool all = true) const;

    virtual QIcon icon() const { return QIcon(":/icons/group"); }

    virtual QString tagName() const { return "group"; }
    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);

private slots:
    void childModified(ProfileItem *item);

private:
    template<class C>
    QList<C *> sortedItems(bool enabled, bool all) const;

    Group * createGroup(const QString &name);
    int addItem(ProfileItem *item);
    int removeItem(ProfileItem *item);
    int removeItem(int index);

    QList<ProfileItem *> m_items;

    QList<Group *> m_groups;
};

#endif // GROUP_H
