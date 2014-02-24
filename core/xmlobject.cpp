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


#include "xmlobject.h"

XmlObject::XmlObject(QObject *parent) :
    QObject(parent)
{
}

XmlObject::~XmlObject()
{
    clear();
}

void XmlObject::clear()
{
    for (int n = 0; n < m_errors.size(); n++)
    {
        XmlError *err = m_errors.at(n);
        if (err)
        {
            delete err;
        }
    }
    m_errors.clear();
}

void XmlObject::addError(int line, int column, const QString &message)
{
    XmlError *err = new XmlError(this);
    err->setLine(line);
    err->setColumn(column);
    err->setMessage(message);
    m_errors.append(err);
}
