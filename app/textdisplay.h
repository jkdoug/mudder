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


#ifndef TEXTDISPLAY_H
#define TEXTDISPLAY_H

#include <QColor>
#include <QString>
#include <QTextBrowser>

class Console;

class TextDisplay : public QTextBrowser
{
    Q_OBJECT
public:
    explicit TextDisplay(QWidget *parent = 0);

    void setConsole(Console *console) { m_console = console; }

    void append(const QColor &fg, const QColor &bg, const QString &text);
    void hyperlink(const QColor &fg, const QColor &bg, const QString &text, const QUrl &url, const QString &tip = QString());

private slots:
    void onAnchorClicked(const QUrl &link);

private:
    Console *m_console;
};

#endif // TEXTDISPLAY_H
