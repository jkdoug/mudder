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


#ifndef ACCELERATOR_H
#define ACCELERATOR_H

#include "executable.h"
#include <QKeySequence>

class XmlError;

class Accelerator : public Executable
{
    Q_OBJECT

    Q_PROPERTY(QKeySequence key READ key WRITE setKey)

public:
    explicit Accelerator(QObject *parent = 0);
    Accelerator(const Accelerator &rhs, QObject *parent = 0);

    Accelerator & operator =(const Accelerator &rhs);

    bool operator ==(const Accelerator &rhs);
    bool operator !=(const Accelerator &rhs);

    bool operator <(const Accelerator &rhs);
    static bool lessSequence(const Accelerator *lhs, const Accelerator *rhs);

    QKeySequence key() const { return m_key; }
    void setKey(const QKeySequence &key) { m_key = key; }

    virtual QIcon icon() const { return QIcon(":/icons/accelerator"); }
    virtual QString value() const { return key().toString(); }
    virtual int sequence() const { return 0; }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);

protected:
    void clone(const Accelerator &rhs);

private:
    QKeySequence m_key;
};

#endif // ACCELERATOR_H
