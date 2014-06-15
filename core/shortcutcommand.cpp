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


#include "shortcutcommand.h"
#include "logger.h"

ShortcutCommand::ShortcutCommand(const QString &text, QShortcut *shortcut, QList<int> contexts, QObject *parent) :
    Command(parent)
{
    Q_ASSERT(shortcut);

    m_userText = text;
    m_shortcut = shortcut;
    m_shortcut->setEnabled(false);
    m_contexts = contexts;
}

void ShortcutCommand::handleKeyChange(const QKeySequence &old)
{
    Q_UNUSED(old);

    m_shortcut->setKey(key());
}

void ShortcutCommand::changeContexts(QList<int> contexts)
{
    bool enabled = false;
    for (int n = 0; n < m_contexts.size(); n++)
    {
        if (contexts.contains(m_contexts.at(n)))
        {
            enabled = true;
            break;
        }
    }

    LOG_TRACE("Context update request on shortcut command", text(), enabled);

    m_shortcut->setEnabled(enabled);
}
