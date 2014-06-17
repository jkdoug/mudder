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


#include "coreapplication.h"
#include "logger.h"
#include <QtDebug>

CoreApplication * CoreApplication::m_instance = 0;

CoreApplication::CoreApplication(int &argc, char **argv) :
    QApplication(argc, argv),
    m_busyCount(0)
{
    if (!m_instance)
    {
        m_instance = this;

        m_contextManager = new ContextManager;
    }
}

CoreApplication::~CoreApplication()
{
    delete m_contextManager;
}

CoreApplication * CoreApplication::instance()
{
    return m_instance;
}

void CoreApplication::setApplicationBusy(bool busy)
{
    instance()->q_setApplicationBusy(busy);
}

void CoreApplication::q_setApplicationBusy(bool busy)
{
    int previous = m_busyCount;

    if (busy)
    {
        m_busyCount++;
    }
    else
    {
        if (m_busyCount > 0)
        {
            m_busyCount--;
        }
        else
        {
            qWarning() << "setApplicationBusy(false) called too many times on CoreApplication";
        }
    }

    if (previous == 0 && m_busyCount == 1)
    {
        emit busyStateChanged(true);
    }
    else if (previous == 1 && m_busyCount == 0)
    {
        emit busyStateChanged(false);
    }
}

bool CoreApplication::applicationBusy()
{
    return instance()->m_busyCount > 0;
}

void CoreApplication::setMainWindow(QWidget *win)
{
    instance()->m_mainWindow = win;
}

QWidget * CoreApplication::mainWindow()
{
    return instance()->m_mainWindow;
}

ContextManager * CoreApplication::contextManager()
{
    return instance()->m_contextManager;
}

bool CoreApplication::notify(QObject *object, QEvent *event)
{
    try
    {
        return QApplication::notify(object, event);
    }
    catch (...)
    {
        LOG_FATAL(tr("CoreApplication caught an unhandled exception..."));
    }
    return false;
}
