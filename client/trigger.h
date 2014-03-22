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


#ifndef TRIGGER_H
#define TRIGGER_H

#include "matchable.h"

class XmlError;

class Trigger : public Matchable
{
    Q_OBJECT

    Q_PROPERTY(bool omit READ omit WRITE setOmit)
    Q_PROPERTY(bool repeat READ repeat WRITE setRepeat)

public:
    explicit Trigger(QObject *parent = 0);
    Trigger(const Trigger &rhs, QObject *parent = 0);

    Trigger & operator =(const Trigger &rhs);

    bool operator ==(const Trigger &rhs);
    bool operator !=(const Trigger &rhs);

    bool operator <(const Trigger &rhs);
    static bool lessSequence(const Trigger *lhs, const Trigger *rhs);

    bool omit() const { return m_omit; }
    void setOmit(bool flag = true) { m_omit = flag; }
    bool repeat() const { return m_repeat; }
    void setRepeat(bool flag = true) { m_repeat = flag; }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);

protected:
    void clone(const Trigger &rhs);

    bool m_omit;
    bool m_repeat;
};

#endif // TRIGGER_H
