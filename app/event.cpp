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


#include "event.h"

extern "C"
{
    #include <src/lauxlib.h>
}

Event::Event(QObject *parent) :
    ProfileItem(parent)
{
    m_reference = LUA_NOREF;
}

Event::Event(const Event &rhs, QObject *parent) :
    ProfileItem(parent)
{
    clone(rhs);
}

Event & Event::operator =(const Event &rhs)
{
    clone(rhs);

    return *this;
}

bool Event::operator ==(const Event &rhs)
{
    if (sequence() != rhs.sequence())
    {
        return false;
    }

    if (m_reference != rhs.m_reference)
    {
        return false;
    }

    return true;
}

bool Event::operator !=(const Event &rhs)
{
    return !(*this == rhs);
}

bool Event::lessSequence(const Event *lhs, const Event *rhs)
{
    return lhs->sequence() < rhs->sequence();
}

void Event::clone(const Event &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    setSequence(rhs.sequence());
    m_reference = rhs.m_reference;
}

void Event::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartElement("event");
    xml.writeAttribute("reference", QString::number(reference()));
    xml.writeAttribute("sequence", QString::number(sequence()));
    xml.writeEndElement();
}
