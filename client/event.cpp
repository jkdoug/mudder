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


#include "event.h"
#include "logger.h"

Event::Event(QObject *parent) :
    Executable(parent)
{
}

Event::Event(const Event &rhs, QObject *parent) :
    Executable(parent)
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
    return Executable::operator ==(rhs);
}

bool Event::operator !=(const Event &rhs)
{
    return !(*this == rhs);
}

bool Event::lessSequence(const Event *lhs, const Event *rhs)
{
    return ProfileItem::lessSequence(lhs, rhs);
}

void Event::clone(const Event &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    Executable::clone(rhs);
}

void Event::toXml(QXmlStreamWriter &xml)
{
    Executable::toXml(xml);
}

void Event::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    Executable::fromXml(xml, errors);

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement())
        {
            break;
        }
    }

    LOG_TRACE("Event::fromXml", fullName());
}
