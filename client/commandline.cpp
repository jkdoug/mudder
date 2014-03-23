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


#include "commandline.h"
#include "coresettings.h"
#include <QRegularExpression>

CommandLine::CommandLine(QWidget *parent) :
    QPlainTextEdit(parent)
{
    SETTINGS->setDefault("CommandLine/ClearAfterSend", false);
    SETTINGS->setDefault("CommandLine/ScriptPrefix", "/");
    SETTINGS->setDefault("CommandLine/Separator", ";");
}

void CommandLine::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (e->modifiers().testFlag(Qt::ShiftModifier))
        {
            textCursor().insertBlock();
            adjustHeight();
        }
        else
        {
            send();
        }
        e->accept();
        return;

    case Qt::Key_Down:
        if (e->modifiers().testFlag(Qt::ControlModifier))
        {
            moveCursor(QTextCursor::Down);
        }
        else
        {
//            historyDown();
            adjustHeight();
        }
        e->accept();
        return;

    case Qt::Key_Up:
        if (e->modifiers().testFlag(Qt::ControlModifier))
        {
            moveCursor(QTextCursor::Up);
        }
        else
        {
//            historyUp();
            adjustHeight();
        }
        e->accept();
        return;

    default:
        // TODO: process accelerators, return if a match is found
        break;
    }

    QPlainTextEdit::keyPressEvent(e);

    adjustHeight();
}

void CommandLine::adjustHeight()
{
    int lineCount = qBound(1, document()->size().toSize().height(), 10);
    int fontHeight = QFontMetrics(font()).height();
    int baseHeight = fontHeight * lineCount;
    int desiredHeight = qMax(baseHeight + fontHeight, 31);

    if (height() != desiredHeight)
    {
        setMinimumHeight(desiredHeight);
        setMaximumHeight(desiredHeight);
    }
}

void CommandLine::send()
{
    QString txt(toPlainText());
    QString prefix(SETTINGS->value("CommandLine/ScriptPrefix").toString());
    if (txt.startsWith(prefix))
    {
        txt = txt.remove(0, prefix.length());
        emit script(txt);
    }
    else if (txt.isEmpty())
    {
        emit command("");
    }
    else
    {
        QString sep(SETTINGS->value("CommandLine/Separator").toString() + QString("\n"));
        QRegularExpression regex("[" + QRegularExpression::escape(sep) + "]");
        QStringList cmds(txt.split(regex, QString::SkipEmptyParts));
        foreach (const QString &cmd, cmds)
        {
            emit command(cmd);
        }
    }

    if (SETTINGS->value("CommandLine/ClearAfterSend").toBool())
    {
        clear();
    }
    else
    {
        selectAll();
    }
}
