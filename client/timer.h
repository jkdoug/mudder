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


#ifndef TIMER_H
#define TIMER_H

#include "executable.h"
#include <QTime>
#include <QTimer>

class XmlError;
class Profile;

class Timer : public Executable
{
    Q_OBJECT

    Q_PROPERTY(QTime interval READ interval WRITE setInterval)
    Q_PROPERTY(int milliseconds READ milliseconds)
    Q_PROPERTY(int firedCount READ firedCount)

public:
    explicit Timer(QObject *parent = 0);
    Timer(const Timer &rhs, QObject *parent = 0);
    ~Timer();

    Timer operator =(const Timer &rhs);

    bool operator ==(const Timer &rhs);
    bool operator !=(const Timer &rhs);

    bool operator <(const Timer &rhs);
    static bool lessInterval(const Timer *lhs, const Timer *rhs);

    void setProfile(Profile *profile);

    virtual void enable(bool flag = true);
    const QTime & interval() const { return m_interval; }
    int milliseconds() const { return m_interval.msec() + 1000 * m_interval.second() + 1000 * 60 * m_interval.minute() + 1000 * 60 * 60 * m_interval.hour(); }
    void setInterval(const QTime &interval);
    bool once() const { return m_once; }
    void setOnce(bool flag = true) { m_once = flag; }
    int firedCount() const { return m_firedCount; }

    virtual QIcon icon() const { return QIcon(":/icons/timer"); }
    virtual QString value() const { return interval().toString(); }
    virtual int sequence() const { return 0; }

    virtual QString tagName() const { return "timer"; }
    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);

public slots:
    void start();
    void stop();
    void fire();

private:
    void clone(const Timer &rhs);

    Profile *m_profile;

    QTime m_interval;
    QTimer *m_timer;

    bool m_once;

    int m_firedCount;
};

#endif // TIMER_H
