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


#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include "textformat.h"
#include "hyperlink.h"
#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QList>
#include <QMultiMap>
#include <QObject>
#include <QPoint>
#include <QStringList>

class Console;

class TextBuffer : public QObject
{
    Q_OBJECT
public:
    TextBuffer(Console *parent = 0);

    void process(const QByteArray &data);
    void command(const QString &str, const QColor &fg, const QColor &bg);
    void echo(const QString &str, const QColor &fg, const QColor &bg);
    int link(const QString &str, const QColor &fg, const QColor &bg, Hyperlink *info);

    Console *console() const;
    int length() const { return m_lines.length(); }
    const QStringList & lines() const { return m_lines; }
    const QByteArray & line() const { return m_line; }
    QString & line(int n) { return m_lines[n]; }
    QList<QDateTime> & times() { return m_times; }
    TextFormat & format(int y, int x) { return m_formats[y][x]; }
    QList<TextFormat> & format(int y) { return m_formats[y]; }
    QList< QList<TextFormat> > & formats() { return m_formats; }
    TextFormat lastFormat() const;
    QStringList lastLines(int n) const;

    bool deleteLine(int line);
    bool deleteLines(int from, int to);
    void clear();

    QList<Hyperlink *> links(int id);

    QString toHtml(const QColor &fg, const QColor &bg, const QPoint &start, const QPoint &end);

    int maxLines() const { return m_maxLines; }
    void setMaxLines(int lines) { m_maxLines = lines; }
    int jumpLines() const { return m_jumpLines; }
    void setJumpLines(int lines) { m_jumpLines = lines; }
    void setSize(int lines, int jump);
    int shrink();
    int wrapWidth() const;

private:
    void processAnsi(int code);
    QColor translateColor(const QString &name);
    void wrap(int startLine);
    int linkId();

    QStringList m_lines;
    QList<QDateTime> m_times;
    QList< QList<TextFormat> > m_formats;

    QByteArray m_line;
    QString m_ansiCode;
    QList<int> m_codes;
    QList<TextFormat> m_format;

    QMultiMap<int, Hyperlink *> m_links;
    int m_linkId;

    bool m_gotESC;
    bool m_gotHeader;

    bool m_waitHighColorMode;
    bool m_fgHighColorMode;
    bool m_bgHighColorMode;
    bool m_isHighColorMode;
    bool m_isDefaultColor;
    bool m_isPrompt;

    QColor m_fgColor;
    QColor m_fgColorBold;
    QColor m_fgColorDefault;
    QColor m_bgColor;
    QColor m_bgColorDefault;

    bool m_bold;
    bool m_italics;
    bool m_underline;

    int m_maxLines;
    int m_jumpLines;
};

#endif // TEXTBUFFER_H
