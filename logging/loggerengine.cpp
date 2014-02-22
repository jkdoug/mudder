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
    m_d = new LoggerEnginePrivate;

    connect(this, SIGNAL(destroyed()), SLOT(finalize()));

    installEventFilter(this);
    enableAllMessageTypes();
}

LoggerEngine::~LoggerEngine()
{
    LOG->detachLoggerEngine(this, false);
    delete m_d;
}

inline Logger::MessageContextFlags LoggerEngine::messageContexts() const
{
    return m_d->messageContexts;
}

inline void LoggerEngine::setMessageContexts(Logger::MessageContextFlags contexts)
{
    m_d->messageContexts = contexts;
}

inline bool LoggerEngine::isInitialized() const
{
    return m_d->isInitialized;
}

inline bool LoggerEngine::isActive() const
{
    return m_d->isActive;
}

inline void LoggerEngine::setActive(bool flag)
{
    m_d->isActive = flag;
}

inline QString LoggerEngine::name() const
{
    return m_d->engineName;
}

void LoggerEngine::setName(const QString &name)
{
    setObjectName(name);
    m_d->engineName = name;
}

inline void LoggerEngine::setEnabledMessageTypes(Logger::MessageTypeFlags types)
{
    m_d->enabledMessageTypes = types;
}

inline Logger::MessageTypeFlags LoggerEngine::getEnabledMessageTypes() const
{
    return m_d->enabledMessageTypes;
}

inline void LoggerEngine::enableAllMessageTypes()
{
    setEnabledMessageTypes(Logger::AllLogLevels);
}

void LoggerEngine::installFormattingEngine(FormattingEngine *engine)
{
    if (engine == m_d->formattingEngine)
    {
        return;
    }

    if (!isFormattingEngineConstant() && m_d->formattingEngine && engine)
    {
        m_d->formattingEngine = engine;
#ifndef QT_NO_DEBUG
        if (m_d->isActive)
        {
            logMessage("Formatting engine change detected.", Logger::Trace);
            logMessage(QString("This engine now logs messages using the following formatting engine: %1").arg(m_d->formattingEngine->name()), Logger::Trace);
        }
#endif
    }
    else if (!m_d->formattingEngine && engine)
    {
        m_d->formattingEngine = engine;
    }
}

FormattingEngine * LoggerEngine::getInstalledFormattingEngine()
{
    return m_d->formattingEngine;
}

QString LoggerEngine::formattingEngineName()
{
    if (m_d->formattingEngine)
    {
        return m_d->formattingEngine->objectName();
    }

    return tr("None");
}

void LoggerEngine::newMessages(const QString &engineName, Logger::MessageType type, Logger::MessageContext context, const QList<QVariant> &messages)
{
    if (!engineName.isEmpty() && engineName != name())
    {
        return;
    }

    if (!m_d->messageContexts.testFlag(context))
    {
        return;
    }

    if (m_d->isActive && m_d->formattingEngine && m_d->enabledMessageTypes.testFlag(type))
    {
        logMessage(m_d->formattingEngine->formatMessage(type, messages), type);
    }
}

inline bool LoggerEngine::removable() const
{
    return m_d->isRemovable;
}

inline void LoggerEngine::setRemovable(bool flag)
{
    m_d->isRemovable = flag;
}
