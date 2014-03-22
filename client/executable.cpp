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


#include "executable.h"
//#include "engine.h"
#include "logger.h"
#include <QElapsedTimer>
#include <QStringList>

extern "C"
{
    #include <lauxlib.h>
}

Executable::Executable(QObject *parent) :
    ProfileItem(parent)
{
    m_reference = LUA_NOREF;
    m_executionCount = 0;
    m_totalTime = 0.0;
    m_averageTime = 0.0;
    m_failed = false;
}

Executable::Executable(const Executable &rhs, QObject *parent) :
    ProfileItem(parent)
{
    clone(rhs);
}

Executable & Executable::operator =(const Executable &rhs)
{
    clone(rhs);

    return *this;
}

bool Executable::operator ==(const Executable &rhs)
{
    if (m_reference != rhs.m_reference)
    {
        return false;
    }

    if (m_contents != rhs.m_contents)
    {
        return false;
    }

    return ProfileItem::operator ==(rhs);
}

bool Executable::operator !=(const Executable &rhs)
{
    return !(*this == rhs);
}

void Executable::clone(const Executable &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    ProfileItem::clone(rhs);

    m_contents = rhs.m_contents;
    m_reference = rhs.m_reference;

    m_failed = rhs.m_failed;

    m_executionCount = rhs.m_executionCount;
    m_totalTime = rhs.m_totalTime;
    m_averageTime = rhs.m_averageTime;
}

bool Executable::enabled() const
{
    return !failed() && ProfileItem::enabled();
}

//bool Executable::execute(Engine *e)
//{
//    if (contents().isEmpty())
//    {
//        return true;
//    }

//    QElapsedTimer timer;
//    timer.start();

//    bool result = e->execute(contents(), this);

//    m_executionCount++;
//    m_totalTime = m_totalTime + timer.elapsed() / 1000.0;
//    m_averageTime = m_totalTime / m_executionCount;

//    return result;
//}

void Executable::toXml(QXmlStreamWriter &xml)
{
    ProfileItem::toXml(xml);

    if (!contents().isEmpty())
    {
        xml.writeTextElement("send", contents());
    }
}

void Executable::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    ProfileItem::fromXml(xml, errors);

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement())
        {
            break;
        }

        if (xml.isStartElement() && xml.name() == "send")
        {
            setContents(xml.readElementText().trimmed());
        }
    }

    LOG_TRACE("Executable::fromXml", fullName());
}
