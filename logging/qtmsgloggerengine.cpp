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


#include "qtmsgloggerengine.h"
#include <QMutex>
#include <QtDebug>

QtMsgLoggerEngine* QtMsgLoggerEngine::m_instance = 0;

QtMsgLoggerEngine* QtMsgLoggerEngine::instance()
{
    static QMutex mutex;
    if (!m_instance)
    {
        mutex.lock();
        if (!m_instance)
        {
            m_instance = new QtMsgLoggerEngine;
        }
        mutex.unlock();
    }

    return m_instance;
}

QtMsgLoggerEngine::QtMsgLoggerEngine() : LoggerEngine()
{
    setName("Qt Message Logger Engine");
}

QtMsgLoggerEngine::~QtMsgLoggerEngine()
{
}

bool QtMsgLoggerEngine::initialize()
{
    m_d->isInitialized = true;
    return true;
}

void QtMsgLoggerEngine::finalize()
{
    m_d->isInitialized = false;
}

QString QtMsgLoggerEngine::description() const
{
    return "Writes log messages to the Qt message system.";
}

QString QtMsgLoggerEngine::status() const
{
    if (m_d->isInitialized)
    {
        if (m_d->isActive)
        {
            return "Logging in progress.";
        }
        return "Ready but inactive.";
    }
    return "Not initialized.";
}

void QtMsgLoggerEngine::logMessage(const QString &message, Logger::MessageType type)
{
    Q_UNUSED(type)
    qDebug() << message;
}
