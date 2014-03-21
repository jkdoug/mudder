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

#include <QDebug>
#include <QObject>
#include <QString>

class XmlError
{
public:
    XmlError(int line = 0, int column = 0, const QString &message = "Undefined error") :
        m_message(message),
        m_line(line),
        m_column(column)
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

    QString toString() const
    {
        QString l;
        if (m_line > 0)
        {
            if (m_column > 0)
            {
                l = QString(": Line %1, Column %2").arg(m_line).arg(m_column);
            }
            else
            {
                l = QString(": Line %1").arg(m_line);
            }
        }

        return QString("XML%1; %2").arg(l).arg(m_message);
    }

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
