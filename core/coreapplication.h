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


#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include "core_global.h"
#include "contextmanager.h"
#include <QApplication>
#include <QWidget>

class CORESHARED_EXPORT CoreApplication : public QApplication
{
    Q_OBJECT
public:
    CoreApplication(int &argc, char ** argv);
    ~CoreApplication();

    static CoreApplication * instance();

    static void setApplicationBusy(bool busy);
    static bool applicationBusy();

    static void setMainWindow(QWidget *win);
    static QWidget * mainWindow();

    static ContextManager * contextManager();

    bool notify(QObject *object, QEvent *event);

private:
    void q_setApplicationBusy(bool busy);

signals:
    void busyStateChanged(bool busy);

private:
    Q_DISABLE_COPY(CoreApplication)

    static CoreApplication *m_instance;

    int m_busyCount;

    QWidget *m_mainWindow;

    ContextManager *m_contextManager;
};

#define CONTEXT_MANAGER CoreApplication::contextManager()

#endif // COREAPPLICATION_H
