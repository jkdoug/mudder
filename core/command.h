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


#ifndef COMMAND_H
#define COMMAND_H

#include "core_global.h"
#include <QKeySequence>
#include <QList>
#include <QObject>
#include <QString>

class QAction;
class QShortcut;

class CORESHARED_EXPORT Command : public QObject
{
    Q_OBJECT
public:
    explicit Command(QList<int> contexts, QObject *parent = 0);

    void setDefaultKey(const QKeySequence &key);
    QKeySequence defaultKey() const { return m_defaultKey; }

    void setKey(const QKeySequence &key);
    QKeySequence key() const { return m_currentKey; }

    QList<int> contexts() const { return m_contexts; }

    virtual QAction * action() const { return 0; }
    virtual QShortcut * shortcut() const { return 0; }
    virtual QString text() const = 0;

    virtual void handleKeyChange(const QKeySequence &old) = 0;

public slots:
    virtual void changeContexts(QList<int> contexts) = 0;

signals:
    void keyChanged();

protected:
    QKeySequence m_defaultKey;
    QKeySequence m_currentKey;

    QList<int> m_contexts;
};

#endif // COMMAND_H
