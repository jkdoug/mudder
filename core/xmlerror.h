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
#include <QObject>
#include <QString>

class CORESHARED_EXPORT XmlError : public QObject
{
    Q_OBJECT
public:
    explicit XmlError(QObject *parent = 0) :
        QObject(parent),
        m_message("Undefined error"),
        m_line(0),
        m_column(0)
    {}

    void setMessage(const QString &message) { m_message = message; }
    const QString & message() const { return m_message; }
    void setLine(int line) { m_line = line; }
    int line() { return m_line; }
    void setColumn(int column) { m_column = column; }
    int column() { return m_column; }

private:
    QString m_message;
    int m_line;
    int m_column;
};

#endif // XMLERROR_H
