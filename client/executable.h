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


#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include "profileitem.h"

class Engine;

class Executable : public ProfileItem
{
    Q_OBJECT

    Q_PROPERTY(QString contents READ contents WRITE setContents)
    Q_PROPERTY(bool failed READ failed WRITE setFailed)
    Q_PROPERTY(int executionCount READ executionCount)
    Q_PROPERTY(double totalTime READ totalTime)
    Q_PROPERTY(double averageTime READ averageTime)

public:
    const QString & contents() const { return m_contents; }
    void setContents(const QString &contents);

    bool failed() const { return m_failed; }
    void setFailed(bool flag = true) { m_failed = flag; }

    int executionCount() const { return m_executionCount; }
    double totalTime() const { return m_totalTime; }
    double averageTime() const { return m_averageTime; }

    virtual bool enabled() const;

    bool execute(Engine *e);

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);

protected:
    explicit Executable(QObject *parent = 0);
    Executable(const Executable &rhs, QObject *parent = 0);

    Executable & operator =(const Executable &rhs);

    bool operator ==(const Executable &rhs);
    bool operator !=(const Executable &rhs);

    void clone(const Executable &rhs);

private:
    QString m_contents;

    bool m_failed;

    int m_executionCount;
    double m_totalTime;
    double m_averageTime;
};

#endif // EXECUTABLE_H
