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


#include "consoledocument.h"
#include "consoledocumentlayout.h"
#include "logger.h"
#include <QTextBlock>
#include <QTextDocumentFragment>

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

ConsoleDocument::ConsoleDocument(QObject *parent) :
    QTextDocument(parent)
{
    m_cursor = new QTextCursor(this);

    m_gotESC = false;
    m_gotHeader = false;

    m_waitHighColorMode = false;
    m_fgHighColorMode = false;
    m_bgHighColorMode = false;
    m_isPrompt = false;

    m_formatCommand.setForeground(Qt::darkYellow);

    m_formatWarning.setForeground(Qt::red);

    m_formatInfo.setForeground(Qt::gray);

    m_formatDefault.setFont(QFont("Consolas", 10));
    m_formatDefault.setForeground(Qt::lightGray);

    m_formatCurrent = m_formatDefault;

    m_cursor->setCharFormat(m_formatCurrent);

    setDocumentLayout(new ConsoleDocumentLayout(this));
}

QString ConsoleDocument::toPlainText(int start, int stop)
{
    QTextCursor cursor(this);
    cursor.setPosition(qMax(start, 0));
    cursor.setPosition(qMin(stop, characterCount() - 1), QTextCursor::KeepAnchor);

    return toPlainText(cursor);
}

QString ConsoleDocument::toPlainText(QTextCursor cur)
{
    if (cur.isNull())
    {
        cur = QTextCursor(this);
        cur.select(QTextCursor::Document);
    }

    return cur.selection().toPlainText();
}

void ConsoleDocument::deleteBlock(int num)
{
    num = qBound(0, num, blockCount() - 1);

    QTextBlock block(findBlockByNumber(num));
    if (!block.isValid())
    {
        return;
    }

    QTextCursor cur(block);
    cur.select(QTextCursor::LineUnderCursor);
    if (cur.isNull() || !cur.hasSelection())
    {
        return;
    }

    cur.removeSelectedText();
}

void ConsoleDocument::process(const QByteArray &data)
{
//    LOG_TRACE("ConsoleDocument::process", data);

    if (m_isPrompt)
    {
        newLine();
    }

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

                m_cursor->insertText(m_text);
                m_text.clear();

                m_cursor->mergeCharFormat(m_formatCurrent);

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
            m_isPrompt = ch == GA;

            m_cursor->insertText(m_text);
            m_text.clear();

//            QTextBlock b(m_cursor->block());
//            LOG_DEBUG("Text line number: " + QString::number(b.blockNumber()));
//            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); it++)
//            {
//                QTextFragment f(it.fragment());
//                if (f.isValid())
//                {
//                    LOG_DEBUG(QString("Text fragment: %1 [%2, %3] - %4").arg(f.text()).arg(f.position()).arg(f.length()).arg(f.charFormat().foreground().color().name()));
//                }
//            }

            emit blockAdded(m_cursor->block(), m_isPrompt);

            if (!m_isPrompt)
            {
                newLine();
            }

            continue;
        }

        m_text.append(ch);
    }
}

void ConsoleDocument::command(const QString &cmd)
{
    appendText(m_formatCommand, cmd);
}

void ConsoleDocument::warning(const QString &msg)
{
    if (m_isPrompt)
    {
        newLine();
    }

    appendText(m_formatWarning, msg);
}

void ConsoleDocument::info(const QString &msg)
{
    if (m_isPrompt)
    {
        newLine();
    }

    appendText(m_formatInfo, msg);
}

void ConsoleDocument::newLine()
{
    m_cursor->insertBlock();
    m_isPrompt = false;
}

void ConsoleDocument::processAnsi(int code)
{
    if (m_waitHighColorMode)
    {
        if (code == 38)
        {
            m_fgHighColorMode = true;
        }
        else if (code == 48)
        {
            m_bgHighColorMode = true;
        }
        else if ((m_fgHighColorMode || m_bgHighColorMode) && code == 5)
        {
            m_waitHighColorMode = true;
        }
        else if (m_fgHighColorMode)
        {
            if (code < 16)
            {
                m_fgHighColorMode = false;
                m_waitHighColorMode = false;

                bool bold = false;
                if (code >= 8)
                {
                    code -= 8;
                    bold = true;
                }

                switch (code)
                {
                case 0:
                    m_formatCurrent.setForeground(translateColor(bold?"lightblack":"black"));
                    break;

                case 1:
                    m_formatCurrent.setForeground(translateColor(bold?"lightred":"red"));
                    break;

                case 2:
                    m_formatCurrent.setForeground(translateColor(bold?"lightgreen":"green"));
                    break;

                case 3:
                    m_formatCurrent.setForeground(translateColor(bold?"lightyellow":"yellow"));
                    break;

                case 4:
                    m_formatCurrent.setForeground(translateColor(bold?"lightblue":"blue"));
                    break;

                case 5:
                    m_formatCurrent.setForeground(translateColor(bold?"lightmagenta":"magenta"));
                    break;

                case 6:
                    m_formatCurrent.setForeground(translateColor(bold?"lightcyan":"cyan"));
                    break;

                case 7:
                    m_formatCurrent.setForeground(translateColor(bold?"lightwhite":"white"));
                    break;
                }
            }
            else if (code < 232)
            {
                code -= 16;

                int r = code / 36;
                int g = (code - r * 36) / 6;
                int b = (code - r * 36) - g * 6;

                m_formatCurrent.setForeground(QColor(r * 42, g * 42, b * 42));
            }
            else
            {
                code = (code - 232) * 10;

                m_formatCurrent.setForeground(QColor(code, code, code));
            }
        }
        else if (m_bgHighColorMode)
        {
            m_bgHighColorMode = false;
            m_waitHighColorMode = false;

            int bold = false;
            if (code >= 8)
            {
                code -= 8;
                bold = true;
            }

            switch (code)
            {
            case 0:
                m_formatCurrent.setBackground(bold?translateColor("lightblack"):QColor());
                break;

            case 1:
                m_formatCurrent.setBackground(translateColor(bold?"lightred":"red"));
                break;

            case 2:
                m_formatCurrent.setBackground(translateColor(bold?"lightgreen":"green"));
                break;

            case 3:
                m_formatCurrent.setBackground(translateColor(bold?"lightyellow":"yellow"));
                break;

            case 4:
                m_formatCurrent.setBackground(translateColor(bold?"lightblue":"blue"));
                break;

            case 5:
                m_formatCurrent.setBackground(translateColor(bold?"lightmagenta":"magenta"));
                break;

            case 6:
                m_formatCurrent.setBackground(translateColor(bold?"lightcyan":"cyan"));
                break;

            case 7:
                m_formatCurrent.setBackground(translateColor(bold?"lightwhite":"white"));
                break;
            }
        }
        else if (code < 232)
        {
            code -= 16;

            int r = code / 36;
            int g = (code - r * 36) / 6;
            int b = (code - r * 36) - g * 6;

            m_formatCurrent.setBackground(QColor(r * 42, g * 42, b * 42));
        }
        else
        {
            code = (code - 232) * 10;

            m_formatCurrent.setBackground(QColor(code, code, code));
        }
    }
    else
    {
        static bool bold = false;
        switch (code)
        {
        case 0:
            m_fgHighColorMode = false;
            m_bgHighColorMode = false;
            m_waitHighColorMode = false;

            bold = false;

            m_formatCurrent = m_formatDefault;
            return;

        case 1:
            bold = true;
            break;

        case 2:
        case 22:
            bold = false;
            break;

        case 3:
        case 23:
            m_formatCurrent.setFontItalic(true);
            break;

        case 4:
        case 24:
            m_formatCurrent.setFontUnderline(true);
            break;

        case 30:
            m_formatCurrent.setForeground(translateColor(bold?"lightblack":"black"));
            break;

        case 31:
            m_formatCurrent.setForeground(translateColor(bold?"lightred":"red"));
            break;

        case 32:
            m_formatCurrent.setForeground(translateColor(bold?"lightgreen":"green"));
            break;

        case 33:
            m_formatCurrent.setForeground(translateColor(bold?"lightyellow":"yellow"));
            break;

        case 34:
            m_formatCurrent.setForeground(translateColor(bold?"lightblue":"blue"));
            break;

        case 35:
            m_formatCurrent.setForeground(translateColor(bold?"lightmagenta":"magenta"));
            break;

        case 36:
            m_formatCurrent.setForeground(translateColor(bold?"lightcyan":"cyan"));
            break;

        case 37:
            m_formatCurrent.setForeground(translateColor(bold?"lightwhite":"white"));
            break;

        case 39:
            m_formatCurrent.setBackground(QColor());
            break;

        case 40:
            m_formatCurrent.setBackground(translateColor("black"));
            break;

        case 41:
            m_formatCurrent.setBackground(translateColor("red"));
            break;

        case 42:
            m_formatCurrent.setBackground(translateColor("green"));
            break;

        case 43:
            m_formatCurrent.setBackground(translateColor("yellow"));
            break;

        case 44:
            m_formatCurrent.setBackground(translateColor("blue"));
            break;

        case 45:
            m_formatCurrent.setBackground(translateColor("magenta"));
            break;

        case 46:
            m_formatCurrent.setBackground(translateColor("cyan"));
            break;

        case 47:
            m_formatCurrent.setBackground(translateColor("white"));
            break;
        }
    }
}

inline QColor ConsoleDocument::translateColor(const QString &name)
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

inline void ConsoleDocument::appendText(const QTextCharFormat &fmt, const QString &text, bool newline)
{
    QTextCharFormat previousFormat(m_cursor->charFormat());

    m_cursor->mergeCharFormat(fmt);

    m_cursor->insertText(text);

    if (newline)
    {
        newLine();
    }

    m_cursor->setCharFormat(previousFormat);
}
