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


#ifndef XMLERROR_H
#define XMLERROR_H

#include "core_global.h"
#include <QDebug>
#include <QObject>
#include <QString>

class CORESHARED_EXPORT XmlError
{
public:
    XmlError() :
        m_message("Undefined error"),
        m_line(0),
        m_column(0)
    {}
    XmlError(const XmlError &rhs)
    {
        m_message = rhs.m_message;
        m_line = rhs.m_line;
        m_column = rhs.m_column;
    }
    ~XmlError() {}

    void setMessage(const QString &message) { m_message = message; }
    const QString & message() const { return m_message; }
    void setLine(int line) { m_line = line; }
    int line() const { return m_line; }
    void setColumn(int column) { m_column = column; }
    int column() const { return m_column; }

private:
    QString m_message;
    int m_line;
    int m_column;
};

inline QDebug operator<<(QDebug dbg, const XmlError &err)
{
    dbg.nospace() << "XmlError(" << err.line() << ":" << err.column();
    dbg.space() << err.message();
    dbg.nospace() << ")";
    return dbg.maybeSpace();
}

#endif // XMLERROR_H
