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


#include "executable.h"
#include "engine.h"
#include "xmlexception.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QStringList>

Executable::Executable(QObject *parent) :
    ProfileItem(parent)
{
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

    m_failed = rhs.m_failed;

    m_executionCount = rhs.m_executionCount;
    m_totalTime = rhs.m_totalTime;
    m_averageTime = rhs.m_averageTime;
}

bool Executable::enabled() const
{
    return !failed() && ProfileItem::enabled();
}

bool Executable::execute(Engine *e)
{
    if (contents().isEmpty())
    {
        return true;
    }

    QElapsedTimer timer;
    timer.start();

    bool result = e->execute(contents(), this);

    m_executionCount++;
    m_totalTime = m_totalTime + timer.elapsed() / 1000.0;
    m_averageTime = m_totalTime / m_executionCount;

    return result;
}

void Executable::toXml(QXmlStreamWriter &xml)
{
    ProfileItem::toXml(xml);

    if (!contents().isEmpty())
    {
        xml.writeTextElement("send", contents());
    }
}

void Executable::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    try
    {
        ProfileItem::fromXml(xml);
    }
    catch (XmlException *xe)
    {
        warnings.append(xe->warnings());

        delete xe;
    }

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

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
