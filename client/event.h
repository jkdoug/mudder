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


#ifndef EVENT_H
#define EVENT_H

#include "matchable.h"

class Event : public Matchable
{
    Q_OBJECT

public:
    explicit Event(QObject *parent = 0);

    virtual QString name() const;
    int reference() const { return m_reference; }
    void setReference(int reference) { m_reference = reference; }

    virtual bool execute(Engine *e, const QVariantList &args = QVariantList());

    virtual QIcon icon() const { return QIcon(":/icons/event"); }
    virtual QString tagName() const { return "event"; }

    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);

private:
    int m_reference;
};

typedef QList<Event *> EventList;

#endif // EVENT_H
