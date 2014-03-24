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


#include "loggerengine.h"
#include "logger.h"

LoggerEngine::LoggerEngine()
{
    m_formattingEngine = 0;

    connect(this, SIGNAL(destroyed()), SLOT(finalize()));

    installEventFilter(this);
    enableAllMessageTypes();
}

LoggerEngine::~LoggerEngine()
{
    LOG->detachLoggerEngine(this, false);
}

void LoggerEngine::setName(const QString &name)
{
    setObjectName(name);
    m_engineName = name;
}

inline void LoggerEngine::enableAllMessageTypes()
{
    setEnabledMessageTypes(Logger::AllLogLevels);
}

void LoggerEngine::installFormattingEngine(FormattingEngine *engine)
{
    if (engine == m_formattingEngine)
    {
        return;
    }

    if (!isFormattingEngineConstant() && m_formattingEngine && engine)
    {
        m_formattingEngine = engine;
#ifndef QT_NO_DEBUG
        if (m_active)
        {
            logMessage("Formatting engine change detected.", Logger::Trace);
            logMessage(QString("This engine now logs messages using the following formatting engine: %1").arg(m_formattingEngine->name()), Logger::Trace);
        }
#endif
    }
    else if (!m_formattingEngine && engine)
    {
        m_formattingEngine = engine;
    }
}

QString LoggerEngine::formattingEngineName()
{
    if (m_formattingEngine)
    {
        return m_formattingEngine->objectName();
    }

    return tr("None");
}

void LoggerEngine::newMessages(const QString &engineName, Logger::MessageType type, Logger::MessageContext context, const QList<QVariant> &messages)
{
    if (!engineName.isEmpty() && engineName != name())
    {
        return;
    }

    if (!m_messageContexts.testFlag(context))
    {
        return;
    }

    if (m_active && m_formattingEngine && m_enabledMessageTypes.testFlag(type))
    {
        logMessage(m_formattingEngine->formatMessage(type, messages), type);
    }
}
