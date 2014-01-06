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


#ifndef ALIAS_H
#define ALIAS_H

#include "matchable.h"
#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Alias : public Matchable
{
    Q_OBJECT

    Q_PROPERTY(bool echo READ echo WRITE setEcho)

public:
    explicit Alias(QObject *parent = 0);
    Alias(const Alias &rhs, QObject *parent = 0);

    Alias & operator =(const Alias &rhs);

    bool operator ==(const Alias &rhs);
    bool operator !=(const Alias &rhs);

    bool operator <(const Alias &rhs);
    static bool lessSequence(const Alias *lhs, const Alias *rhs);

    bool echo() const { return m_echo; }
    void setEcho(bool flag = true) { m_echo = flag; }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

private:
    void clone(const Alias &rhs);

    bool m_echo;
};

#endif // ALIAS_H
