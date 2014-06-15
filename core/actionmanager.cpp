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


#include "actionmanager.h"
#include "logger.h"
#include "coreapplication.h"
#include "proxyaction.h"
#include "shortcutcommand.h"

ActionManager::ActionManager(QObject *parent) :
    QObject(parent)
{
}

ActionContainer * ActionManager::createMenu(const QString &id, bool &existed)
{
    existed = m_containers.contains(id);

    if (existed)
    {
        return m_containers[id];
    }

    ActionContainer *container = new ActionContainer(id, this);
    if (container)
    {
        m_containers.insert(id, container);
    }

    return container;
}

ActionContainer * ActionManager::menu(const QString &id)
{
    return m_containers.value(id, 0);
}

Command * ActionManager::registerAction(const QString &id, QAction *action, const QList<int> &context)
{
    if (!action)
    {
        return 0;
    }

    // Check if there is already a front end action for this action id:
    Command *command = m_commands.value(id, 0);
    if (!command)
    {
        command = registerActionPlaceHolder(id, action->text());
    }
    Q_ASSERT(command);

    ProxyAction* multi = qobject_cast<ProxyAction*>(command);
    if (multi)
    {
        multi->setObjectName(id);

        if (multi->defaultKey().isEmpty())
        {
            multi->setDefaultKey(action->shortcut());
        }

        if (action->shortcut() != multi->key() && !multi->key().isEmpty() && !action->shortcut().isEmpty())
        {
            LOG_ERROR(tr("Conflicting shortcut found for command %1 when trying to register shortcut %2. Original shortcut of %3 will be used.").arg(action->text()).arg(action->shortcut().toString()).arg(multi->key().toString()));
        }

        if (multi->key().isEmpty() && !action->shortcut().isEmpty())
        {
            LOG_TRACE(tr("Base action shortcut did not exist previously, now using shortcut (%1) from backend action: %2.").arg(action->shortcut().toString()).arg(action->text()));

            multi->setKey(action->shortcut());
        }

        LOG_TRACE(tr("Registering new backend action for base action %1 (shortcut %2). New action: %3 (shortcut %4).").arg(multi->text()).arg(multi->key().toString()).arg(action->text()).arg(action->shortcut().toString()));

        action->setShortcut(QKeySequence());

        multi->addAction(action, context);

        return multi;
    }

    return 0;
}

Command * ActionManager::registerActionPlaceHolder(const QString &id, const QString &text, const QKeySequence &key,
                                                   const QList<int> &context, const QIcon &icon)
{
    if (m_commands.contains(id))
    {
        LOG_ERROR(tr("Attempting to register action place holder for a command which already exists with ID: %1").arg(id));

        return command(id);
    }

    QAction *frontendAction = 0;
    if (text.isEmpty())
    {
        frontendAction = new QAction(id.section(".", -1), 0);
    }
    else
    {
        frontendAction = new QAction(text, 0);
    }

    frontendAction->setIcon(icon);
    frontendAction->setObjectName(id);
    frontendAction->setShortcutContext(Qt::ApplicationShortcut);

    if (!CoreApplication::mainWindow())
    {
        LOG_WARNING(tr("CoreApplication::mainWindow() is required when registering actions in the action manager. Proxy actions will not work as intended."));
    }
    else
    {
        CoreApplication::mainWindow()->addAction(frontendAction);
    }

    ProxyAction *newAction = new ProxyAction(frontendAction);
    if (newAction)
    {
        newAction->setDefaultText(id);

        if (!context.isEmpty())
        {
            QAction* backendAction = new QAction(frontendAction->text(), 0);
            newAction->addAction(backendAction, context);
        }

        newAction->changeContexts(CONTEXT_MANAGER->activeContexts());
        m_commands.insert(id, newAction);

        newAction->setKey(key);
        newAction->setDefaultKey(key);

        return newAction;
    }

    delete frontendAction;

    return 0;
}

Command * ActionManager::registerShortcut(const QString &id, const QString &text, QShortcut *shortcut,
                                          const QList<int> &activeContexts)
{
    if (!shortcut)
    {
        return 0;
    }

    if (m_commands.contains(id))
    {
        LOG_ERROR(tr("Attempting to register shortcut for a command which already exists with ID: %1").arg(id));

        return command(id);
    }

    QList<int> contexts(activeContexts);
    if (contexts.isEmpty())
    {
        contexts << CONTEXT_MANAGER->contextId(CONTEXT_STANDARD);
    }

    ShortcutCommand *newShortcut = new ShortcutCommand(text, shortcut, contexts, CoreApplication::mainWindow());
    if (newShortcut)
    {
        newShortcut->setDefaultText(id);
        newShortcut->changeContexts(CONTEXT_MANAGER->activeContexts());
        m_commands.insert(id, newShortcut);

        newShortcut->setDefaultKey(shortcut->key());
        newShortcut->setKey(shortcut->key());

        return newShortcut;
    }

    return 0;
}

void ActionManager::unregisterCommandsForContext(int context)
{
    // TODO
//    foreach (Command *command, m_commands)
//    {
//        command->unregisterContext(context);
//    }
}

Command * ActionManager::command(const QString &id) const
{
    return m_commands.value(id, 0);
}

ActionContainer * ActionManager::actionContainer(const QString &id) const
{
    return m_containers.value(id, 0);
}

void ActionManager::restoreDefaultShortcuts()
{
    foreach (Command *command, m_commands)
    {
        command->setKey(command->defaultKey());
    }
}

void ActionManager::handleContextChanged(QList<int> newContexts)
{
//    foreach (Command *command, m_commands)
//    {
//        command->setCurrentContext(newContexts);
//    }
}
