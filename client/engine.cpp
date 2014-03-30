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


#include "engine.h"
#include <QMessageBox>

Engine::Engine(QObject *parent) :
    QObject(parent)
{
    m_global = 0;
}

Engine::~Engine()
{
    if (m_global)
    {
        lua_close(m_global);
        m_global = 0;
    }
}

void Engine::initialize()
{
    m_global = luaL_newstate();
    if (!m_global)
    {
        QMessageBox mb;
        mb.setIcon(QMessageBox::Critical);
        mb.setWindowTitle(tr("Engine Trouble"));
        mb.setText(tr("Failed to initialize Lua API."));
        mb.exec();
        return;
    }

    luaL_openlibs(m_global);

    lua_settop(m_global, 0);

    emit output("Script engine initialized.");
}
