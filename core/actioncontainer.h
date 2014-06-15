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


#ifndef ACTIONCONTAINER_H
#define ACTIONCONTAINER_H

#include <QAction>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <QString>
#include "core_global.h"
#include "command.h"

class CORESHARED_EXPORT ActionContainer : public QObject
{
    Q_OBJECT
public:
    explicit ActionContainer(const QString &name, QObject *parent = 0);
    ~ActionContainer();

    virtual QMenu *menu() const { return m_menu; }

    virtual void addAction(Command *action, const QString &before = QString());
    virtual void addSeparator(const QString &before = QString());
    virtual void addMenu(ActionContainer *menu, const QString &before = QString());

private:
    QMenu *m_menu;
    QList<QPointer<ActionContainer> > m_subMenus;
    QMap<QString, QAction*> m_actionMap;
};

#endif // ACTIONCONTAINER_H
