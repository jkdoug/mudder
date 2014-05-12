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
#include "xmlerror.h"

Event::Event(QObject *parent) :
    Executable(parent)
{
    m_evalCount = 0;
    m_matchCount = 0;
}

bool Event::match(const QString &str)
{
    m_evalCount++;

    if (m_title.compare(str) == 0)
    {
        m_lastMatched = QDateTime::currentDateTime();
        m_matchCount++;
        return true;
    }

    return false;
}

void Event::toXml(QXmlStreamWriter &xml)
{
    xml.writeAttribute("title", m_title);

    Executable::toXml(xml);
}

void Event::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    QString title(xml.attributes().value("title").toString());
    if (title.isEmpty())    // TODO: more validation
    {
        errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or empty 'title' attribute in event"));
    }
    setTitle(title);

    Executable::fromXml(xml, errors);

    if (contents().isEmpty())
    {
        errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or empty 'send' element in event"));
    }
}
