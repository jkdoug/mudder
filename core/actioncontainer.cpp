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


#include "actioncontainer.h"

ActionContainer::ActionContainer(const QString &name, QObject *parent) :
    QObject(parent)
{
    m_menu = new QMenu(name);
}

ActionContainer::~ActionContainer()
{
    delete m_menu;
}

void ActionContainer::addAction(Command *command, const QString &before)
{
    if (!command)
    {
        return;
    }

    m_actionMap.insert(command->defaultText(), command->action());

    if (m_actionMap.contains(before))
    {
        m_menu->insertAction(m_actionMap[before], command->action());
        return;
    }

    m_menu->addAction(command->action());
}

void ActionContainer::addSeparator(const QString &before)
{
    if (m_actionMap.contains(before))
    {
        m_menu->insertSeparator(m_actionMap[before]);
        return;
    }

    m_menu->addSeparator();
}

void ActionContainer::addMenu(ActionContainer *menu, const QString &before)
{
    if (!menu)
    {
        return;
    }

    if (m_actionMap.contains(before))
    {
        menu->setParent(this);
        m_menu->insertMenu(m_actionMap[before], menu->menu());
        m_subMenus.prepend(menu);
        return;
    }

    m_menu->addMenu(menu->menu());
}
