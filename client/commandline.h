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


#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QCompleter>
#include <QPlainTextEdit>
#include <QStringList>
#include <QStringListModel>
#include <QVariant>

class CommandLine : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CommandLine(QWidget *parent = 0);

    void setAccelerated(bool flag = true) { m_accelerated = flag; }

public slots:
    void optionChanged(const QString &key, const QVariant &val);
    void echoToggled(bool flag);

signals:
    void command(const QString &cmd);
    void script(const QString &cmd);
    void accelerator(const QKeySequence &key);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void focusInEvent(QFocusEvent *e);

private slots:
    void addToHistory(const QString &cmd);
    void insertCompletion(const QString &completion);

private:
    void adjustHeight();
    void historyUp();
    void historyDown();
    QString textUnderCursor() const;
    void updateCompletions();
    void processCommand(const QString &text);

    QStringList m_history;
    int m_historyPosition;

    QCompleter *m_completer;
    QStringListModel *m_completionModel;

    bool m_escapeClears;
    bool m_clearCommandLine;
    bool m_echoOn;
    bool m_accelerated;

    QString m_scriptPrefix;
    QString m_commandSeparator;
};

#endif // COMMANDLINE_H
