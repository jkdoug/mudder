/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redidataibute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is didataibuted in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "textbuffer.h"
#include "console.h"
#include <QDebug>
#include <QMap>

static const QLatin1Char ESC('\x1B');
static const QLatin1Char ANSI_START('[');
static const QLatin1Char ANSI_SEPARATOR(';');
static const QLatin1Char ANSI_END('m');
static const QLatin1Char GA('\xFF');
static const QLatin1Char CR('\r');
static const QLatin1Char LF('\n');
static const QString CRLF(QString(CR) + QString(LF));
static const QString CRLFGA(CRLF + GA);
static const QString DIGITS("0123456789");


TextBuffer::TextBuffer(Console *parent) :
    QObject(parent)
{
    m_linkId = 0;

    m_gotESC = false;
    m_gotHeader = false;

    m_waitHighColorMode = false;
    m_fgHighColorMode = false;
    m_bgHighColorMode = false;
    m_isHighColorMode = false;
    m_isDefaultColor = false;
    m_isPrompt = false;

    m_fgColor = QColor(Qt::lightGray);
    m_fgColorBold = QColor(Qt::white);
    m_bgColor = QColor(Qt::black);
    m_fgColorDefault = m_fgColor;
    m_bgColorDefault = m_bgColor;

    m_bold = false;
    m_italics = false;
    m_underline = false;

    m_maxLines = 10000;
    m_jumpLines = 100;
}

Console *TextBuffer::console() const
{
    return qobject_cast<Console *>(parent());
}

void TextBuffer::process(const QByteArray &data)
{
    const QDateTime timePacket(QDateTime::currentDateTime());

    for (int pos = 0; pos < data.length(); pos++)
    {
        uchar ch = (uchar)data.at(pos);
        if (ch == ESC)
        {
            m_gotESC = true;
            continue;
        }
        else if (m_gotESC && ch == ANSI_START)
        {
            m_gotESC = false;
            m_gotHeader = true;
            continue;
        }
        else if (m_gotHeader)
        {
            if (DIGITS.contains(ch))
            {
                m_ansiCode.append(ch);
            }
            else if (ch == ANSI_SEPARATOR)
            {
                m_codes.append(m_ansiCode.toInt());
                m_ansiCode.clear();
            }
            else if (ch == ANSI_END)
            {
                m_gotHeader = false;

                m_codes.append(m_ansiCode.toInt());
                m_ansiCode.clear();


                foreach(int code, m_codes)
                {
                    processAnsi(code);
                }

                m_codes.clear();
            }
            else
            {
                m_ansiCode.clear();
                m_gotHeader = false;
            }
            continue;
        }

        if (m_gotHeader)
        {
            return;
        }

        if (CRLFGA.contains(ch))
        {
            m_times.append(timePacket);
            m_formats.append(m_format);
            m_lines.append(m_line);

            m_line.clear();
            m_format.clear();

            m_isPrompt = ch == GA;

            int line = length() - 1;
            if (console()->processTriggers(m_lines.back()))
            {
                wrap(line);

                if (length() >= maxLines())
                {
                    shrink();
                }
            }
            else
            {
                deleteLine(line);

            }
            continue;
        }

        m_line.append(ch);
        m_format.append(TextFormat(!m_isDefaultColor && m_bold?m_fgColorBold:m_fgColor,
                                   m_bgColor,
                                   m_isDefaultColor?m_bold:false,
                                   m_italics,
                                   m_underline));
    }
}

void TextBuffer::processAnsi(int code)
{
    if (m_waitHighColorMode)
    {
        if (code == 38)
        {
            m_isHighColorMode = true;
            m_fgHighColorMode = true;
        }
        else if (code == 48)
        {
            m_isHighColorMode = true;
            m_bgHighColorMode = true;
        }
        else if (m_isHighColorMode && code == 5)
        {
            m_waitHighColorMode = true;
        }
        else if (m_fgHighColorMode)
        {
            if (code < 16)
            {
                m_fgHighColorMode = false;
                m_waitHighColorMode = false;
                m_isHighColorMode = false;
                m_isDefaultColor = false;

                if (code >= 8)
                {
                    code -= 8;
                    m_bold = true;
                }
                else
                {
                    m_bold = false;
                }

                switch (code)
                {
                case 0:
                    m_fgColor = translateColor("black");
                    m_fgColorBold = translateColor("lightblack");
                    break;

                case 1:
                    m_fgColor = translateColor("red");
                    m_fgColorBold = translateColor("lightred");
                    break;

                case 2:
                    m_fgColor = translateColor("green");
                    m_fgColorBold = translateColor("lightgreen");
                    break;

                case 3:
                    m_fgColor = translateColor("yellow");
                    m_fgColorBold = translateColor("lightyellow");
                    break;

                case 4:
                    m_fgColor = translateColor("blue");
                    m_fgColorBold = translateColor("lightblue");
                    break;

                case 5:
                    m_fgColor = translateColor("magenta");
                    m_fgColorBold = translateColor("lightmagenta");
                    break;

                case 6:
                    m_fgColor = translateColor("cyan");
                    m_fgColorBold = translateColor("lightcyan");
                    break;

                case 7:
                    m_fgColor = translateColor("white");
                    m_fgColorBold = translateColor("lightwhite");
                    break;
                }
            }
            else if (code < 232)
            {
                code -= 16;

                int r = code / 36;
                int g = (code - r * 36) / 6;
                int b = (code - r * 36) - g * 6;

                m_fgColor = QColor(r * 42, g * 42, b * 42);
                m_fgColorBold = m_fgColor;
            }
            else
            {
                code = (code - 232) * 10;

                m_fgColor = QColor(code, code, code);
                m_fgColorBold = m_fgColor;
            }
        }
        else if (m_bgHighColorMode)
        {
            m_bgHighColorMode = false;
            m_waitHighColorMode = false;
            m_isHighColorMode = false;
            m_isDefaultColor = false;

            int bold = false;
            if (code >= 8)
            {
                code -= 8;
                bold = true;
            }

            QColor bgColorBold;
            switch (code)
            {
            case 0:
                m_bgColor = translateColor("black");
                bgColorBold = translateColor("lightblack");
                break;

            case 1:
                m_bgColor = translateColor("red");
                bgColorBold = translateColor("lightred");
                break;

            case 2:
                m_bgColor = translateColor("green");
                bgColorBold = translateColor("lightgreen");
                break;

            case 3:
                m_bgColor = translateColor("yellow");
                bgColorBold = translateColor("lightyellow");
                break;

            case 4:
                m_bgColor = translateColor("blue");
                bgColorBold = translateColor("lightblue");
                break;

            case 5:
                m_bgColor = translateColor("magenta");
                bgColorBold = translateColor("lightmagenta");
                break;

            case 6:
                m_bgColor = translateColor("cyan");
                bgColorBold = translateColor("lightcyan");
                break;

            case 7:
                m_bgColor = translateColor("white");
                bgColorBold = translateColor("lightwhite");
                break;
            }

            if (bold && bgColorBold.isValid())
            {
                m_bgColor = bgColorBold;
            }
        }
        else if (code < 232)
        {
            code -= 16;

            int r = code / 36;
            int g = (code - r * 36) / 6;
            int b = (code - r * 36) - g * 6;

            m_bgColor = QColor(r * 42, g * 42, b * 42);
        }
        else
        {
            code = (code - 232) * 10;

            m_bgColor = QColor(code, code, code);
        }
    }
    else
    {
        switch (code)
        {
        case 0:
            m_fgHighColorMode = false;
            m_bgHighColorMode = false;
            m_waitHighColorMode = false;
            m_isHighColorMode = false;
            m_isDefaultColor = true;

            m_fgColor = m_fgColorDefault;
            m_bgColor = m_bgColorDefault;

            m_bold = false;
            m_italics = false;
            m_underline = false;
            break;

        case 1:
            m_bold = true;
            break;

        case 2:
        case 22:
            m_bold = false;
            break;

        case 3:
        case 23:
            m_italics = true;
            break;

        case 4:
        case 24:
            m_underline = true;
            break;

        case 30:
            m_fgColor = translateColor("black");
            m_fgColorBold = translateColor("lightblack");
            m_isDefaultColor = false;
            break;

        case 31:
            m_fgColor = translateColor("red");
            m_fgColorBold = translateColor("lightred");
            m_isDefaultColor = false;
            break;

        case 32:
            m_fgColor = translateColor("green");
            m_fgColorBold = translateColor("lightgreen");
            m_isDefaultColor = false;
            break;

        case 33:
            m_fgColor = translateColor("yellow");
            m_fgColorBold = translateColor("lightyellow");
            m_isDefaultColor = false;
            break;

        case 34:
            m_fgColor = translateColor("blue");
            m_fgColorBold = translateColor("lightblue");
            m_isDefaultColor = false;
            break;

        case 35:
            m_fgColor = translateColor("magenta");
            m_fgColorBold = translateColor("lightmagenta");
            m_isDefaultColor = false;
            break;

        case 36:
            m_fgColor = translateColor("cyan");
            m_fgColorBold = translateColor("lightcyan");
            m_isDefaultColor = false;
            break;

        case 37:
            m_fgColor = translateColor("white");
            m_fgColorBold = translateColor("lightwhite");
            m_isDefaultColor = false;
            break;

        case 39:
            m_bgColor = m_bgColorDefault;
            break;

        case 40:
            m_bgColor = translateColor("black");
            break;

        case 41:
            m_bgColor = translateColor("red");
            break;

        case 42:
            m_bgColor = translateColor("green");
            break;

        case 43:
            m_bgColor = translateColor("yellow");
            break;

        case 44:
            m_bgColor = translateColor("blue");
            break;

        case 45:
            m_bgColor = translateColor("magenta");
            break;

        case 46:
            m_bgColor = translateColor("cyan");
            break;

        case 47:
            m_bgColor = translateColor("white");
            break;
        }
    }
}

void TextBuffer::command(const QString &str, const QColor &fg, const QColor &bg)
{
    QList<TextFormat> format;
    for (int n = 0; n < str.length(); n++)
    {
        TextFormat fmt(fg, bg, false, false, false);
        format.append(fmt);
    }

    if (m_isPrompt)
    {
        m_isPrompt = false;
        m_lines[length() - 1].append(str);
        m_formats[length() - 1].append(format);
        return;
    }

    m_isPrompt = false;
    m_lines.append(str);
    m_times.append(QDateTime::currentDateTime());
    m_formats.append(format);
}

void TextBuffer::echo(const QString &str, const QColor &fg, const QColor &bg)
{
    QColor fgColor(fg);
    QColor bgColor(bg);

    if (!fgColor.isValid())
    {
        fgColor = lastFormat().foreground();
    }
    if (!bgColor.isValid())
    {
        bgColor = lastFormat().background();
    }

    const QDateTime timePacket(QDateTime::currentDateTime());
    for (int pos = 0; pos < str.length(); pos++)
    {
        if (CRLF.contains(str[pos]))
        {
            m_lines.append(m_line);
            m_times.append(timePacket);
            m_formats.append(m_format);

            wrap(length() - 1);

            m_line.clear();
            m_format.clear();
            continue;
        }

        m_line.append(str[pos]);
        m_format.append(TextFormat(fgColor, bgColor, false, false, false));
    }

    m_isPrompt = (!str.endsWith(CR) && !str.endsWith(LF));
}

int TextBuffer::link(const QString &str, const QColor &fg, const QColor &bg, Hyperlink *info)
{
    Q_ASSERT(info != 0);

    int id = linkId();
    QList<TextFormat> format;
    for (int n = 0; n < str.length(); n++)
    {
        TextFormat fmt(fg, bg, false, false, true, id);
        format.append(fmt);
    }

    info->setParent(this);
    m_links.insertMulti(id, info);

    if (m_isPrompt)
    {
        m_isPrompt = false;
        m_lines[length() - 1].append(str);
        m_formats[length() - 1].append(format);
    }
    else
    {
        m_lines.append(str);
        m_times.append(QDateTime::currentDateTime());
        m_formats.append(format);
    }

    return id;
}

TextFormat TextBuffer::lastFormat() const
{
    if (m_format.count() > 0)
    {
        return m_format.back();
    }
    else if (m_formats.count() > 0 && m_formats.back().count() > 0)
    {
        return m_formats.back().back();
    }

    return TextFormat(m_fgColorDefault, m_bgColorDefault, false, false, false);
}

QStringList TextBuffer::lastLines(int n) const
{
    int count = qBound(0, n, m_lines.count());
    if (count < 1)
    {
        return QStringList();
    }

    return lines().mid(lines().count() - count);
}

bool TextBuffer::deleteLine(int line)
{
    return deleteLines(line, line);
}

bool TextBuffer::deleteLines(int from, int to)
{
    if (from < 0 || from > to || from >= m_lines.count() ||
        to < 0 || to >= m_lines.count())
    {
        return false;
    }

    int delta = to - from + 1;
    for (int line = from; line < from + delta; line++)
    {
        m_lines.removeAt(line);
        m_times.removeAt(line);
        m_formats.removeAt(line);
    }

    return true;
}

void TextBuffer::clear()
{
    m_lines.clear();
    m_times.clear();
    m_formats.clear();
    m_isPrompt = false;
}

QList<Hyperlink *> TextBuffer::links(int id)
{
    return m_links.values(id);
}

QString TextBuffer::toHtml(const QColor &fg, const QColor &bg, const QPoint &start, const QPoint &end)
{
    QString text;
    if (start.y() < 0 || end.y() < 0 ||
        start.y() >= length() || end.y() >= length())
    {
        return text;
    }

    bool bold = false;
    bool italics = false;
    bool underline = false;
    bool opened = false;
    QColor background(bg);
    QColor foreground(fg);
    for (int y = start.y(); y <= end.y(); y++)
    {
        if (y >= length())
        {
            break;
        }

        int from = 0;
        int to = line(y).length();
        if (y == start.y())
        {
            from = start.x();
        }
        else if (y == end.y())
        {
            to = end.x();
        }

        for (int x = from; x < to; x++)
        {
            TextFormat fmt(format(y, x));
            if (fmt.isBold() != bold ||
                fmt.isItalics() != italics ||
                fmt.isUnderline() != underline ||
                fmt.background().name() != background.name() ||
                fmt.foreground().name() != foreground.name())
            {
                if (opened)
                {
                    text.append("</span>");
                }

                bold = fmt.isBold();
                italics = fmt.isItalics();
                underline = fmt.isUnderline();
                background = fmt.background();
                foreground = fmt.foreground();

                QString style("<span style='");
                if (background.name() != bg.name())
                {
                    style.append(QString("background: %1; ").arg(background.name()));
                }
                if (foreground.name() != fg.name())
                {
                    style.append(QString("color: %1; ").arg(foreground.name()));
                }
                if (bold)
                {
                    style.append("font-weight: bold; ");
                }
                if (italics)
                {
                    style.append("font-style: italics; ");
                }
                if (underline)
                {
                    style.append("font-decoration: underline; ");
                }
                style.append("'>");

                if (style.contains(":"))
                {
                    text.append(style);
                    opened = true;
                }
            }

            QChar ch(line(y).at(x));
            switch (ch.toLatin1())
            {
            case '<':
                text.append("&lt;");
                break;

            case '>':
                text.append("&gt;");
                break;

            case '&':
                text.append("&amp;");
                break;

            case '\'':
                text.append("&apos;");
                break;

            case '\"':
                text.append("&quot;");
                break;

            default:
                text.append(ch);
                break;
            }
        }

        if (opened)
        {
            text.append("</span>");

            bold = false;
            italics = false;
            underline = false;
            background = bg;
            foreground = fg;

            opened = false;
        }
        text.append("\n");
    }

    if (opened)
    {
        text.append("</span>");
    }

    return text;
}

void TextBuffer::setSize(int lines, int jump)
{
    lines = qBound(100, lines, 1000000);
    jump = qBound(10, jump, lines - 1);

    setMaxLines(lines);
    setJumpLines(jump);
}

int TextBuffer::shrink()
{
    int jump = qBound(1, jumpLines(), length());
    deleteLines(0, jump);
    return jump;
}

int TextBuffer::wrapWidth() const
{
    return console()->profile()->wrapColumn();
}

inline QColor TextBuffer::translateColor(const QString &name)
{
    static QMap<QString, QColor> colors;
    if (colors.count() < 1)
    {
        colors["black"] = QColor(Qt::black);
        colors["lightblack"] = QColor(Qt::darkGray);
        colors["red"] = QColor(Qt::darkRed);
        colors["lightred"] = QColor(Qt::red);
        colors["green"] = QColor(Qt::darkGreen);
        colors["lightgreen"] = QColor(Qt::green);
        colors["blue"] = QColor(Qt::darkBlue);
        colors["lightblue"] = QColor(Qt::blue);
        colors["yellow"] = QColor(Qt::darkYellow);
        colors["lightyellow"] = QColor(Qt::yellow);
        colors["cyan"] = QColor(Qt::darkCyan);
        colors["lightcyan"] = QColor(Qt::cyan);
        colors["magenta"] = QColor(Qt::darkMagenta);
        colors["lightmagenta"] = QColor(Qt::magenta);
        colors["white"] = QColor(Qt::lightGray);
        colors["lightwhite"] = QColor(Qt::white);
    }

    if (!colors.contains(name))
    {
        return QColor();
    }

    return colors[name];
}

inline void TextBuffer::wrap(int startLine)
{
    if (startLine >= length() || startLine < 0)
    {
        return;
    }

    QStringList lines;
    QList< QList<TextFormat> > formats;
    QList<QDateTime> times;

    int lineCount = 0;
    for (int n = startLine; n < length(); n++)
    {
        QString line(m_lines[n]);
        int len = line.length();

        // Skip lines that are already shorter than the wrap width
        if (len < wrapWidth())
        {
            lines.append(m_lines[n]);
            formats.append(m_formats[n]);
            times.append(m_times[n]);

            lineCount++;
            continue;
        }

        QList<TextFormat> format(m_formats[n]);
        QDateTime time(m_times[n]);

        int lastBreak = -1;
        int start = 0;
        int x = 0;
        bool isBreakable = false;
        bool wasBreakable = false;
        bool isParens = false;
        bool wasParens = false;

        for (int n = 0; n < len; n++)
        {
            const QChar c(line[n]);

            isParens = (c == QLatin1Char('(') || c == QLatin1Char('[') || c == QLatin1Char('{'));
            isBreakable = (c.isSpace() || c.isSymbol()) && !isParens;

            if (!isBreakable && n < len - 1)
            {
                const QChar c2 = line[n + 1];
                isBreakable = (c2 == QLatin1Char('(') || c2 == QLatin1Char('[') || c2 == QLatin1Char('{'));
            }

            if (c == QLatin1Char('/') && (wasBreakable || wasParens))
            {
                isBreakable = false;
            }

            int breakAt = -1;
            if (x + 1 > console()->profile()->wrapColumn() && lastBreak != -1)
            {
                breakAt = lastBreak;
            }

            if (x + 1 > console()->profile()->wrapColumn() - 4 && lastBreak == -1)
            {
                breakAt = n;
            }

            if (n == len - 2 && x + 2 > console()->profile()->wrapColumn())
            {
                breakAt = (lastBreak == -1)?(n - 1):lastBreak;
            }

            if (breakAt != -1)
            {
                int end = breakAt;
                lines.append(line.mid(start, end - start + 1));
                formats.append(format.mid(start, end - start + 1));
                times.append(time);
                start = end + 1;

                x = 0;
                wasBreakable = true;
                wasParens = false;
                if (lastBreak != -1)
                {
                    n = lastBreak;
                    lastBreak = -1;
                    continue;
                }
            }
            else if (isBreakable)
            {
                lastBreak = n;
            }

            x++;
            wasBreakable = isBreakable;
            wasParens = isParens;
        }
        lineCount++;
        if (start < len)
        {
            lines.append(line.mid(start));
            formats.append(format.mid(start));
            times.append(time);
        }
    }

    for (int n = 0; n < lineCount; n++)
    {
        m_lines.pop_back();
        m_formats.pop_back();
        m_times.pop_back();
    }

    m_lines.append(lines);
    m_formats.append(formats);
    m_times.append(times);
}

int TextBuffer::linkId()
{
    m_linkId++;

    if (m_linkId > 5000000)
    {
        m_linkId = 1;
    }

    return m_linkId;
}
