/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QCompleter>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QWidget>

class Console;

class CommandLine : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CommandLine(QWidget *parent = 0);

    void setConsole(Console *console) { m_console = console; }

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void focusInEvent(QFocusEvent *e);

signals:
    void command(const QString &cmd);
    void script(const QString &code);

private slots:
    void insertCompletion(const QString &completion);

private:
    Console * console() { return m_console; }

    QString textUnderCursor() const;
    void updateCompletions();

    void send();

    void historyUp();
    void historyDown();

    void adjustHeight();

private:
    Console *m_console;

    QStringList m_history;
    int m_historyPosition;

    QCompleter *m_completer;
    QStringListModel *m_completionModel;
};

#endif // COMMANDLINE_H
