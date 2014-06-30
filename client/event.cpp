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


#include "event.h"
#include <QElapsedTimer>
#include "engine.h"
#include "xmlerror.h"

extern "C"
{
    #include "lauxlib.h"
}

Event::Event(QObject *parent) :
    Matchable(parent)
{
    m_reference = LUA_NOREF;
}

QString Event::name() const
{
    if (m_reference != LUA_NOREF)
    {
        return QString("!anonHandler[%1]").arg(m_reference);
    }

    return m_name;
}

bool Event::execute(Engine *e, const QVariantList &args)
{
    if (m_reference == LUA_NOREF)
    {
        return Executable::execute(e, args);
    }

    QElapsedTimer timer;
    timer.start();

    bool result = e->execute(m_reference, this, args);

    m_executionCount++;
    m_totalTime = m_totalTime + timer.elapsed() / 1000.0;
    m_averageTime = m_totalTime / m_executionCount;

    return result;
}

void Event::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    Matchable::fromXml(xml, errors);

    if (contents().isEmpty())
    {
        errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or empty 'send' element in event"));
    }
}
