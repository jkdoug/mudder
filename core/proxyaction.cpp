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


#include "proxyaction.h"
#include "logger.h"
#include "coreapplication.h"

ProxyAction::ProxyAction(QAction *action, QObject *parent) :
    QObject(parent),
    m_initialized(false)
{
    m_action = action;

    if (m_action)
    {
        m_action->setEnabled(false);
        m_action->setParent(this);

        m_backup = new QAction(0);
        m_backup->setIcon(m_action->icon());
        m_backup->setIconText(m_action->iconText());

        m_backup->setText(m_action->text());
        m_backup->setStatusTip(m_action->statusTip());
        m_backup->setWhatsThis(m_action->whatsThis());

        m_backup->setCheckable(m_action->isCheckable());
        m_backup->setEnabled(m_action->isEnabled());
        m_backup->setVisible(m_action->isVisible());

        bool block = m_backup->blockSignals(true);
        m_backup->setChecked(m_action->isChecked());
        m_backup->blockSignals(block);
    }
}

ProxyAction::~ProxyAction()
{
    if (m_action)
    {
        delete m_action;
    }

    if (m_backup)
    {
        delete m_backup;
    }
}

void ProxyAction::setDefaultKey(const QKeySequence &key)
{
    m_defaultKey = key;
    handleKeyChange(m_currentKey);
    emit keyChanged();
}

void ProxyAction::setKey(const QKeySequence &key)
{
    QKeySequence old(m_currentKey);
    m_currentKey = key;
    handleKeyChange(old);
    emit keyChanged();
}

QString ProxyAction::text() const
{
    if (m_action)
    {
        return m_action->text();
    }

    return QString();
}

void ProxyAction::handleKeyChange(const QKeySequence &old)
{
    QString oldTooltip(QString("<span style=\"color: gray; font-size: small\">%1</span>").arg(old.toString(QKeySequence::NativeText)));
    QList<QPointer<QAction> > actions(m_contextActions.values());
    QAction *backendAction = 0;

    for (int n = 0; n < actions.count(); n++)
    {
        backendAction = actions.at(n);
        if (backendAction)
        {
            if (backendAction->toolTip().endsWith(oldTooltip))
            {
                QString chopped(backendAction->toolTip());
                chopped.chop(oldTooltip.length());
                backendAction->setToolTip(chopped);
            }
        }
    }

    if (m_currentKey.isEmpty() && !m_defaultKey.isEmpty())
    {
        m_currentKey = m_defaultKey;
    }

    m_action->setShortcut(m_currentKey);

    QString newTooltip(QString("<span style=\"color: gray; font-size: small\">%1</span>").arg(key().toString(QKeySequence::NativeText).toHtmlEscaped()));
    QString activeTooltip(m_activeAction ? m_activeAction->toolTip() : m_backup->toolTip());
    if (m_action->shortcut().isEmpty())
    {
        m_action->setToolTip(activeTooltip);
    }
    else
    {
        m_action->setToolTip(activeTooltip.trimmed() + " " + newTooltip);

        for (int n = 0; n < actions.count(); n++)
        {
            backendAction = actions.at(n);
            if (backendAction)
            {
                backendAction->setToolTip(backendAction->toolTip().trimmed() + " " + newTooltip);
            }
        }
    }
}

void ProxyAction::addAction(QAction *action, QList<int> contexts)
{
    if (!action)
    {
        return;
    }

    if (contexts.isEmpty())
    {
        if (m_contextActions.contains(0))
        {
            LOG_WARNING(QString("Attempting to register an action for a multi-context (\"Standard Context\") action twice. Last action will be ignored: %1")
                        .arg(action->text()));
            return;
        }

        m_contextActions.insert(0, action);
    }
    else
    {
        for (int n = 0; n < contexts.count(); n++)
        {
            int context = contexts.at(n);
            if (m_contextActions.contains(context))
            {
                LOG_WARNING(QString("Attempting to register a backend action for a proxy action twice for a single context with name: %1. Last action will be ignored: %2")
                            .arg(CONTEXT_MANAGER->contextName(context))
                            .arg(action->text()));
                continue;
            }

            m_contextActions.insert(context, action);

            if (action->objectName().isEmpty())
            {
                if (action->text().isEmpty())
                {
                    action->setObjectName(objectName());
                }
                else
                {
                    action->setObjectName(action->text());
                }
            }
        }
    }

    changeContexts(CONTEXT_MANAGER->activeContexts());
}

void ProxyAction::changeContexts(QList<int> contexts)
{
    LOG_TRACE("Context update request on proxy action", text());

    if (m_contextActions.isEmpty())
    {
        LOG_TRACE("No backend actions stored here.");
        return;
    }

    m_contexts = contexts;

    QAction *oldAction = m_activeAction;
    m_activeAction = 0;

    for (int n = 0; n < m_contexts.size(); n++)
    {
        QAction *a = m_contextActions.value(m_contexts.at(n), 0);
        if (a)
        {
            m_activeAction = a;
            m_activeAction->setObjectName(a->text());

            LOG_TRACE(QString("Backend action found: %1, shortcut: %2, proxy shortcut: %3")
                      .arg(m_activeAction->text())
                      .arg(m_activeAction->shortcut().toString())
                      .arg(m_action->shortcut().toString()));
            break;
        }
    }

    if (m_activeAction == oldAction && m_initialized)
    {
        updateFrontend();

        LOG_TRACE("New backend action is the same as the active action; nothing to be done.");
        return;
    }

    if (oldAction)
    {
        LOG_TRACE(QString("Disconnecting multi-context action from previous backend action in parent: %1")
                  .arg(oldAction->parent() ? oldAction->parent()->objectName() : "Unspecified parent"));

        disconnect(oldAction, SIGNAL(changed()), this, SLOT(updateFrontend()));
        disconnect(m_action, SIGNAL(triggered(bool)), oldAction, SIGNAL(triggered(bool)));
        disconnect(m_action, SIGNAL(toggled(bool)), oldAction, SLOT(setChecked(bool)));
    }

    if (m_activeAction)
    {
        LOG_TRACE(QString("Connecting base action: %1, shortcut: %2, parent: %3")
                  .arg(m_activeAction->text())
                  .arg(m_action->shortcut().toString())
                  .arg(m_activeAction->parent() ? m_activeAction->parent()->objectName() : "Unspecified parent"));

        connect(m_activeAction, SIGNAL(changed()), SLOT(updateFrontend()));
        connect(m_action, SIGNAL(triggered(bool)), m_activeAction, SIGNAL(triggered(bool)));
        connect(m_action, SIGNAL(toggled(bool)), m_activeAction, SLOT(setChecked(bool)));

        updateFrontend();

        m_initialized = true;

        return;
    }
    else
    {
        LOG_TRACE("New backend action could not be found; action will be disabled in this context.");
    }

    m_action->setEnabled(false);
}

void ProxyAction::updateFrontend()
{
    if (m_contextActions.isEmpty() || !m_activeAction || !m_action)
    {
        return;
    }

    m_action->setIcon(m_activeAction->icon().isNull() ? m_backup->icon() : m_activeAction->icon());
    m_action->setIconText(m_activeAction->iconText().isEmpty() ? m_backup->iconText() : m_activeAction->iconText());
    m_action->setText(m_activeAction->text().isEmpty() ? m_backup->text() : m_activeAction->text());
    m_action->setStatusTip(m_activeAction->statusTip().isEmpty() ? m_backup->statusTip() : m_activeAction->statusTip());
    m_action->setWhatsThis(m_activeAction->whatsThis().isEmpty() ? m_backup->whatsThis() : m_activeAction->whatsThis());

    m_action->setCheckable(m_activeAction->isCheckable());
    m_action->setEnabled(m_activeAction->isEnabled());
    m_action->setVisible(m_activeAction->isVisible());

    bool block = m_action->blockSignals(true);
    m_action->setChecked(m_activeAction->isChecked());
    m_action->blockSignals(block);
}
