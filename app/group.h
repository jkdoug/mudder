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


#ifndef GROUP_H
#define GROUP_H

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "profileitem.h"
#include "accelerator.h"
#include "alias.h"
#include "timer.h"
#include "trigger.h"
#include "variable.h"

class Group : public ProfileItem
{
    Q_OBJECT

    Q_PROPERTY(QList<Group *> groups READ groups)
    Q_PROPERTY(QList<Accelerator *> accelerators READ accelerators)
    Q_PROPERTY(QList<Alias *> aliases READ aliases)
    Q_PROPERTY(QList<Timer *> timers READ timers)
    Q_PROPERTY(QList<Trigger *> triggers READ triggers)
    Q_PROPERTY(QList<Variable *> variables READ variables)
    Q_PROPERTY(QString path READ path)

public:
    explicit Group(QObject *parent = 0);
    Group(const Group &rhs, QObject *parent = 0);

    Group & operator =(const Group &rhs);

    bool operator <(const Group &rhs);

    void merge(const Group &rhs);

    const QList<Group *> & groups() const { return m_groups; }
    const QList<Accelerator *> & accelerators() const { return m_accelerators; }
    const QList<Alias *> & aliases() const { return m_aliases; }
    const QList<Timer *> & timers() const { return m_timers; }
    const QList<Trigger *> & triggers() const { return m_triggers; }
    const QList<Variable *> & variables() const { return m_variables; }

    void addGroup(Group *group);
    void addAccelerator(Accelerator *accelerator);
    void addAlias(Alias *alias);
    void addTimer(Timer *timer);
    void addTrigger(Trigger *trigger);
    void addVariable(Variable *variable);

    bool deleteGroup(Group *group);
    bool deleteAccelerator(Accelerator *accelerator);
    bool deleteAlias(Alias *alias);
    bool deleteTimer(Timer *timer);
    bool deleteTrigger(Trigger *trigger);
    bool deleteVariable(Variable *variable);

    const QStringList & warnings();

    QString path() const;

    virtual int sequence() const { return 0; }

    const QList<Accelerator *> & sortedAccelerators();
    const QList<Alias *> & sortedAliases();
    const QList<Timer *> & sortedTimers();
    const QList<Trigger *> & sortedTriggers();
    const QList<Variable *> & sortedVariables();

    virtual void toXml(QXmlStreamWriter &xml, const QString &element);
    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

private:
    void clone(const Group &rhs);

    QList<Group *> m_groups;
    QList<Accelerator *> m_accelerators;
    QList<Alias *> m_aliases;
    QList<Timer *> m_timers;
    QList<Trigger *> m_triggers;
    QList<Variable *> m_variables;

    QList<Accelerator *> m_sortedAccelerators;
    QList<Alias *> m_sortedAliases;
    QList<Timer *> m_sortedTimers;
    QList<Trigger *> m_sortedTriggers;
    QList<Variable *> m_sortedVariables;

    bool m_sortAccelerators;
    bool m_sortAliases;
    bool m_sortTimers;
    bool m_sortTriggers;
    bool m_sortVariables;

    QStringList m_warnings;
};

#endif // GROUP_H
