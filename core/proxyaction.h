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


#ifndef PROXYACTION_H
#define PROXYACTION_H

#include "core_global.h"
#include <QAction>
#include <QHash>
#include <QPointer>

class CORESHARED_EXPORT ProxyAction : public QObject
{
    Q_OBJECT
public:
    explicit ProxyAction(QAction *action, QObject *parent = 0);
    ~ProxyAction();

    void setDefaultKey(const QKeySequence &key);
    QKeySequence defaultKey() const { return m_defaultKey; }

    void setKey(const QKeySequence &key);
    QKeySequence key() const { return m_currentKey; }

    void setDefaultText(const QString &text) { m_defaultText = text; }
    QString defaultText() const { return m_defaultText; }

    virtual QAction * action() const { return m_action; }
    virtual QString text() const;

    virtual void handleKeyChange(const QKeySequence &old);

    void addAction(QAction *action, QList<int> contexts);

signals:
    void keyChanged();

public slots:
    virtual void changeContexts(QList<int> contexts);

private slots:
    void updateFrontend();

private:
    QKeySequence m_defaultKey;
    QKeySequence m_currentKey;

    QString m_defaultText;

    QAction *m_action;
    QAction *m_backup;

    bool m_initialized;

    QPointer<QAction> m_activeAction;
    QHash<int, QPointer<QAction> > m_contextActions;

    QList<int> m_contexts;
};

#endif // PROXYACTION_H
