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


#include "timer.h"
#include "profile.h"
#include "xmlexception.h"
#include <QDebug>
#include <QStringList>

Timer::Timer(QObject *parent) :
    Executable(parent)
{
    m_profile = 0;

    m_interval.setHMS(0, 0, 0);
    m_once = false;
    m_timer = new QTimer(this);

    m_firedCount = 0;

    connect(m_timer, SIGNAL(timeout()), this, SLOT(fire()));
}

Timer::Timer(const Timer &rhs, QObject *parent) :
    Executable(parent)
{
    m_timer = new QTimer(this);

    clone(rhs);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(fire()));
}

Timer::~Timer()
{
    delete m_timer;
}

bool Timer::operator ==(const Timer &rhs)
{
    if (m_interval != rhs.m_interval)
    {
        return false;
    }

    if (m_once != rhs.m_once)
    {
        return false;
    }

    return Executable::operator ==(rhs);
}

bool Timer::operator !=(const Timer &rhs)
{
    return !(*this == rhs);
}

Timer Timer::operator =(const Timer &rhs)
{
    clone(rhs);

    return *this;
}

void Timer::clone(const Timer &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    Executable::clone(rhs);

    m_profile = rhs.m_profile;

    m_once = rhs.m_once;

    m_firedCount = rhs.m_firedCount;

    setInterval(rhs.interval());
}

bool Timer::operator <(const Timer &rhs)
{
    return interval() < rhs.interval();
}

bool Timer::lessInterval(const Timer *lhs, const Timer *rhs)
{
    return lhs->interval() < rhs->interval();
}

void Timer::setProfile(Profile *profile)
{
    m_profile = profile;

    if (m_profile && enabled())
    {
        start();
    }
    else
    {
        stop();
    }
}

void Timer::enable(bool flag)
{
    // Don't restart a running timer (or stop a dead timer)
    if (enabledFlag() == flag)
    {
        return;
    }

    Executable::enable(flag);

    if (enabled())
    {
        start();
    }
    else
    {
        stop();
    }
}

void Timer::setInterval(const QTime &interval)
{
    Q_ASSERT(m_timer != 0);

    m_interval = interval;
    m_timer->setInterval(milliseconds());
    if (enabled())
    {
        start();
    }
    else
    {
        stop();
    }
}

void Timer::start()
{
    Q_ASSERT(m_timer != 0);

    if (!m_profile)
    {
        return;
    }

    if (milliseconds() < 1)
    {
        qDebug() << "Timer has no interval";
        return;
    }

    m_timer->start();
}

void Timer::stop()
{
    Q_ASSERT(m_timer != 0);
    m_timer->stop();
}

void Timer::fire()
{
    if (!m_profile || !m_profile->engine())
    {
        return;
    }

    m_firedCount++;

    Group *previousGroup = m_profile->activeGroup();
    Q_ASSERT(previousGroup != 0);
    m_profile->setActiveGroup(group());

    if (!execute(m_profile->engine()))
    {
        enable(false);
    }

    m_profile->setActiveGroup(previousGroup);

    if (once())
    {
        m_profile->deleteTimer(this);
    }
}

void Timer::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartElement("timer");

    if (interval().hour() > 0)
    {
        xml.writeAttribute("h", QString::number(interval().hour()));
    }
    if (interval().minute() > 0)
    {
        xml.writeAttribute("m", QString::number(interval().minute()));
    }
    if (interval().second() > 0)
    {
        xml.writeAttribute("s", QString::number(interval().second()));
    }
    if (interval().msec() > 0)
    {
        xml.writeAttribute("ms", QString::number(interval().msec()));
    }

    if (once())
    {
        xml.writeAttribute("once", "y");
    }

    Executable::toXml(xml);

    xml.writeEndElement();
}

void Timer::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    int h = xml.attributes().value("h").toInt(0, 0);
    int m = xml.attributes().value("m").toInt(0, 0);
    int s = xml.attributes().value("s").toInt(0, 0);
    int ms = xml.attributes().value("ms").toInt(0, 0);

    QTime time(h, m, s, ms);
    if (!time.isValid() || (h <= 0 && m <= 0 && s <= 0 && ms <= 0))
    {
        warnings.append(tr("XML: Line %1; invalid interval for timer").arg(xml.lineNumber()));
    }
    setInterval(time);

    setOnce(xml.attributes().value("once").compare("y", Qt::CaseInsensitive) == 0);

    try
    {
        Executable::fromXml(xml);
    }
    catch (XmlException *xe)
    {
        warnings.append(xe->warnings());

        delete xe;
    }

    if (contents().isEmpty())
    {
        warnings.append(tr("XML: Line %1; missing or empty 'send' element in timer").arg(xml.lineNumber()));
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
