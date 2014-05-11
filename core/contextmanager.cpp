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


#include "contextmanager.h"
#include "logger.h"

ContextManager::ContextManager(QObject *parent) :
    QObject(parent),
    m_counter(-1)
{
    setContext(registerContext(CONTEXT_STANDARD));
}

int ContextManager::registerContext(const QString &name)
{
    LOG_TRACE("ContextManager::registerContext", name);

    int id = contextId(name);
    if (id == -1)
    {
        return -1;
    }

    if (!m_contexts.contains(id))
    {
        m_contexts.prepend(id);
        LOG_DEBUG(QString("Context Manager: Registering new context: %1 with ID %2.").arg(name).arg(id));
    }

    return id;
}

bool ContextManager::unregisterContext(int id, bool notify)
{
    if (!hasContext(id))
    {
        return false;
    }

    if (notify)
    {
        emit unregisteringContext(id);
    }

    if (m_activeContexts.contains(id))
    {
        removeContext(id, notify);
    }

    m_contexts.removeOne(id);

    emit unregisteredContext(id);

    return true;
}

bool ContextManager::unregisterContext(const QString &name, bool notify)
{
    return unregisterContext(contextId(name), notify);
}

bool ContextManager::hasContext(QList<int> ids) const
{
    for (int n = 0; n < m_contexts.count(); n++)
    {
        if (ids.contains(m_contexts.at(n)))
        {
            return true;
        }
    }

    return false;
}

bool ContextManager::hasContext(int id) const
{
    return m_contexts.contains(id);
}

bool ContextManager::hasContext(const QString &name) const
{
    return m_contextMap.contains(name);
}

void ContextManager::setContext(int id, bool notify)
{
    if (m_activeContexts.contains(id) && !notify)
    {
        LOG_DEBUG("Context already active; the following contexts are currently active:");
        for (int n = 0; n < m_activeContexts.size(); n++)
        {
            LOG_DEBUG(QString("- %1 - ID: %2, Name: %3").arg(n).arg(m_activeContexts.at(n)).arg(contextName(m_activeContexts.at(n))));
        }
        return;
    }

    if (notify)
    {
        emit settingContext(id);
    }

    m_activeContexts.clear();
    if (id != 0)
    {
        m_activeContexts.append(0);
    }

    if (m_contexts.contains(id))
    {
        m_activeContexts.append(id);
        LOG_DEBUG("Context set; the following contexts are currently active:");
        for (int n = 0; n < m_activeContexts.size(); n++)
        {
            LOG_DEBUG(QString("- %1 - ID: %2, Name: %3").arg(n).arg(m_activeContexts.at(n)).arg(contextName(m_activeContexts.at(n))));
        }
    }
    else
    {
        LOG_WARNING(QString("Attempting to set unregistered context in function setContext with ID: %1").arg(id));
    }

    if (notify)
    {
        emit newContext(id);
        emit contextChanged(activeContexts());
    }
}

void ContextManager::appendContext(int id, bool notify)
{
    if (m_contexts.contains(id))
    {
        if (m_activeContexts.contains(id))
        {
            LOG_DEBUG("Context already active; the following contexts are currently active:");
            for (int n = 0; n < m_activeContexts.size(); n++)
            {
                LOG_DEBUG(QString("- %1 - ID: %2, Name: %3").arg(n).arg(m_activeContexts.at(n)).arg(contextName(m_activeContexts.at(n))));
            }
            return;
        }

        if (notify)
        {
            emit appendingContext(id);
        }

        m_activeContexts.append(id);

        LOG_DEBUG("Context appended; the following contexts are currently active:");
        for (int n = 0; n < m_activeContexts.size(); n++)
        {
            LOG_DEBUG(QString("- %1 - ID: %2, Name: %3").arg(n).arg(m_activeContexts.at(n)).arg(contextName(m_activeContexts.at(n))));
        }

        if (notify)
        {
            emit appendedContext(id);
            emit contextChanged(activeContexts());
        }
    }
    else
    {
        LOG_ERROR(QString("Attempting to append unregistered context in function appendContext with ID: %1").arg(id));
    }
}

void ContextManager::removeContext(int id, bool notify)
{
    if (id == contextId(CONTEXT_STANDARD))
    {
        return;
    }

    for (int n = 0; n < m_activeContexts.count(); n++)
    {
        if (m_activeContexts.at(n) == id)
        {
            emit removingContext(id);
            m_activeContexts.removeAt(n);

            LOG_DEBUG("Context removed; the following contexts are currently active:");
            for (int n = 0; n < m_activeContexts.size(); n++)
            {
                LOG_DEBUG(QString("- %1 - ID: %2, Name: %3").arg(n).arg(m_activeContexts.at(n)).arg(contextName(m_activeContexts.at(n))));
            }

            if (notify)
            {
                emit removedContext(id);
                emit contextChanged(activeContexts());
            }
        }
    }
}

void ContextManager::setContext(const QString &name, bool notify)
{
    LOG_DEBUG(QString("Clearing all contexts. New active context: %1").arg(name));
    setContext(contextId(name), notify);
}

void ContextManager::appendContext(const QString &name, bool notify)
{
    LOG_DEBUG(QString("Appending context: %1").arg(name));
    appendContext(contextId(name), notify);
}

void ContextManager::removeContext(const QString &name, bool notify)
{
    LOG_DEBUG(QString("Removing context: %1").arg(name));
    removeContext(contextId(name), notify);
}

void ContextManager::broadcastState()
{
    emit contextChanged(activeContexts());
}

int ContextManager::contextId(const QString &name)
{
    if (name.isEmpty())
    {
        return -1;
    }

    if (m_contextMap.contains(name))
    {
        return m_contextMap.value(name);
    }

    m_contextMap[name] = ++m_counter;
    return m_counter;
}

QString ContextManager::contextName(int id)
{
    return m_contextMap.key(id);
}
