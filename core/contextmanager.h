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


#ifndef CONTEXTMANAGER_H
#define CONTEXTMANAGER_H

#include "core_global.h"
#include <QList>
#include <QMap>
#include <QObject>

class CORESHARED_EXPORT ContextManager : public QObject
{
    Q_OBJECT
public:
    explicit ContextManager(QObject *parent = 0);

    int registerContext(const QString &name);
    bool unregisterContext(int id, bool notify = true);
    bool unregisterContext(const QString &name, bool notify = true);

    bool hasContext(QList<int> ids) const;
    bool hasContext(int id) const;
    bool hasContext(const QString &name) const;

    void setContext(int id, bool notify = true);
    void appendContext(int id, bool notify = true);
    void removeContext(int id, bool notify = true);
    void setContext(const QString &name, bool notify = true);
    void appendContext(const QString &name, bool notify = true);
    void removeContext(const QString &name, bool notify = true);

    void broadcastState();

    int contextId(const QString &name);
    QString contextName(int id);

    QList<int> activeContexts() const { return m_activeContexts; }

signals:
    void unregisteringContext(int id);
    void unregisteredContext(int id);
    void settingContext(int id);
    void newContext(int id);
    void appendingContext(int id);
    void appendedContext(int id);
    void removingContext(int id);
    void removedContext(int id);
    void contextChanged(QList<int> ids);

private:
    int m_counter;
    QMap<QString, int> m_contextMap;
    QList<int> m_contexts;
    QList<int> m_activeContexts;
};

#define CONTEXT_STANDARD "lib.core.Context.Standard"

#endif // CONTEXTMANAGER_H
