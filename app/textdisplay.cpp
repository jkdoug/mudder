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


#include <QDebug>
#include <QDesktopServices>
#include "textdisplay.h"
#include "console.h"
#include "engine.h"

TextDisplay::TextDisplay(QWidget *parent) :
    QTextBrowser(parent)
{
    setOpenLinks(false);
    setOpenExternalLinks(false);

    m_console = 0;

    connect(this, SIGNAL(anchorClicked(QUrl)), this, SLOT(onAnchorClicked(QUrl)));
}

void TextDisplay::append(const QColor &fg, const QColor &bg, const QString &text)
{
    QTextCharFormat fmt;
    fmt.setForeground(fg);
    fmt.setBackground(bg);

    textCursor().insertText(text, fmt);
    ensureCursorVisible();
}

void TextDisplay::hyperlink(const QColor &fg, const QColor &bg, const QString &text, const QUrl &url, const QString &tip)
{
    QTextCharFormat prev(currentCharFormat());

    QTextCharFormat fmt;
    fmt.setForeground(fg);
    fmt.setBackground(bg);
    fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
//    fmt.setAnchor(true);
    fmt.setAnchorNames(QStringList() << tip);
    fmt.setAnchorHref(url.toString());

    textCursor().insertText(text, fmt);

    setCurrentCharFormat(prev);
}

void TextDisplay::onAnchorClicked(const QUrl &link)
{
    if (!m_console)
    {
        return;
    }

    if (link.scheme().compare("alias") == 0)
    {
        m_console->commandEntered(link.host());
    }
    else if (link.scheme().compare("lua") == 0)
    {
        m_console->engine()->execute(link.port());
    }
    else
    {
        QDesktopServices::openUrl(link);
    }
}
