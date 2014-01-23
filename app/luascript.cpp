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


#include "luascript.h"
#include "luahighlighter.h"
#include <QApplication>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>

LuaScript::LuaScript(QWidget *parent) :
    CodeEditor(parent)
{
    setSyntaxHighlighter(new LuaHighlighter());
}

bool LuaScript::load(const QString &filename)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QApplication::restoreOverrideCursor();
        return false;
    }

    QByteArray contents(file.readAll());
    if (file.error() != QFile::NoError)
    {
        QApplication::restoreOverrideCursor();
        return false;
    }

    setFilename(filename);
    setWindowTitle(QFileInfo(filename).fileName());

    document()->setPlainText(contents);
    document()->setModified(false);

    moveCursor(QTextCursor::Start);

    QApplication::restoreOverrideCursor();
    return true;
}

bool LuaScript::save(const QString &filename)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QApplication::restoreOverrideCursor();
        return false;
    }

    if (file.write(document()->toPlainText().toLocal8Bit()) < 0)
    {
        QApplication::restoreOverrideCursor();
        return false;
    }

    setWindowTitle(QFileInfo(filename).fileName());

    document()->setModified(false);

    QApplication::restoreOverrideCursor();
    return true;
}
