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


#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QMap>
#include <QObject>
#include "core_global.h"
#include "actioncontainer.h"

class CORESHARED_EXPORT ActionManager : public QObject
{
    Q_OBJECT
public:
    explicit ActionManager(QObject *parent = 0);

    virtual ActionContainer * createMenu(const QString &id, bool& existed);
    virtual ActionContainer * menu(const QString &id);

    virtual Command * registerAction(const QString &id, QAction *action,
                                     const QList<int> &context = QList<int>());
    virtual Command * registerActionPlaceHolder(const QString &id, const QString& text,
                                                const QKeySequence &key = QKeySequence(),
                                                const QList<int> &context = QList<int>(),
                                                const QIcon &icon = QIcon());
    virtual Command * registerShortcut(const QString &id, const QString& text, QShortcut *shortcut,
                                       const QList<int> &activeContexts = QList<int>());

    virtual void unregisterCommandsForContext(int context);
    virtual Command * command(const QString &id) const;
    virtual ActionContainer * actionContainer(const QString &id) const;
    virtual void restoreDefaultShortcuts();

public slots:
    virtual void handleContextChanged(QList<int> newContexts);

private:
    QMap<QString, Command*> m_commands;
    QMap<QString, ActionContainer*> m_containers;
};

#endif // ACTIONMANAGER_H
