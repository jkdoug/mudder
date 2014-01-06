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


#ifndef EVENT_H
#define EVENT_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QXmlStreamWriter>
#include "profileitem.h"

class Event : public ProfileItem
{
    Q_OBJECT

    Q_PROPERTY(int reference READ reference)

public:
    explicit Event(QObject *parent = 0);
    Event(const Event &rhs, QObject *parent = 0);

    Event & operator =(const Event &rhs);

    bool operator ==(const Event &rhs);
    bool operator !=(const Event &rhs);

    static bool lessSequence(const Event *lhs, const Event *rhs);

    int reference() const { return m_reference; }
    void setReference(int reference) { m_reference = reference; }

    virtual void toXml(QXmlStreamWriter &xml);

private:
    void clone(const Event &rhs);

    int m_reference;
};

typedef QMap<QString, QList<Event *> > EventMap;

#endif // EVENT_H
