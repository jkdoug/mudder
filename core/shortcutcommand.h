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


#ifndef SHORTCUTCOMMAND_H
#define SHORTCUTCOMMAND_H

#include "core_global.h"
#include "command.h"
#include <QShortcut>

class CORESHARED_EXPORT ShortcutCommand : public Command
{
    Q_OBJECT
public:
    explicit ShortcutCommand(const QString &text, QShortcut *shortcut, QList<int> contexts, QObject *parent = 0);

    virtual QShortcut * shortcut() const { return m_shortcut; }
    virtual QString text() const { return m_userText; }

    virtual void handleKeyChange(const QKeySequence &old);

public slots:
    virtual void changeContexts(QList<int> contexts);

private:
    QString m_userText;

    QShortcut *m_shortcut;
};

#endif // SHORTCUTCOMMAND_H
