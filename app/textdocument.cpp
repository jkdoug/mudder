#include "textdocument.h"
#include "textblockdata.h"
#include <QDateTime>
#include <QDebug>
#include <QLatin1Char>
#include <QMap>
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

TextDocument::TextDocument(QObject *parent) :
    QTextDocument(parent)
{
    m_selected = false;

    m_cursor = new QTextCursor(this);

    m_gotESC = false;
    m_gotHeader = false;

    m_waitHighColorMode = false;
    m_fgHighColorMode = false;
    m_bgHighColorMode = false;
    m_isPrompt = false;

    m_formatDefault.setFont(QFont("Consolas", 10));
    m_formatDefault.setForeground(Qt::lightGray);

    m_formatCurrent = m_formatDefault;

    m_cursor->setCharFormat(m_formatCurrent);
}

void TextDocument::setFont(const QFont &font)
{
    m_formatDefault.setFont(font);
    setDefaultFont(font);

    m_formatCurrent.setFont(font);
    m_cursor->mergeCharFormat(m_formatCurrent);
}

void TextDocument::setCommandColor(const QColor &fg, const QColor &bg)
{
    m_formatCommand.setForeground(fg);
    m_formatCommand.setBackground(bg);
}

QString TextDocument::toPlainText(int start, int stop)
{
    QTextCursor cursor(this);
    cursor.setPosition(qMax(start, 0));
    cursor.setPosition(qMin(stop, characterCount() - 1), QTextCursor::KeepAnchor);

    return toPlainText(cursor);
}

QString TextDocument::toPlainText(const QTextCursor &cur)
{
    if (cur.isNull())
    {
        return m_cursor->selection().toPlainText();
    }

    return cur.selection().toPlainText();
}

QString TextDocument::toHtml(int start, int stop, const QColor &fg, const QColor &bg, const QFont &font)
{
    QTextCursor cursor(this);
    cursor.setPosition(qMax(start, 0));
    cursor.setPosition(qMin(stop, characterCount() - 1), QTextCursor::KeepAnchor);

    return toHtml(cursor, fg, bg, font);
}

QString TextDocument::toHtml(const QTextCursor &cur, const QColor &fg, const QColor &bg, const QFont &font)
{
    QTextCursor cursor;
    if (cur.isNull())
    {
        cursor = *m_cursor;
    }
    else
    {
        cursor = cur;
    }

    QString text;
    if (!cursor.hasSelection())
    {
        return text;
    }

    bool bold = false;
    bool italics = false;
    bool underline = false;
    bool opened = false;

    QColor background(bg);
    QColor foreground(fg);
    QFont fonted(font);

    int selectionStart = cursor.selectionStart();
    int selectionEnd = cursor.selectionEnd();

    cursor.setPosition(selectionStart);
    while (cursor.position() < selectionEnd && !cursor.atEnd())
    {
        QChar ch(characterAt(cursor.position()));

        cursor.setPosition(cursor.position() + 1);
        if (ch.toLatin1() == 0)
        {
            continue;
        }

        QTextCharFormat fmt(cursor.charFormat());
        bool boldNow = fmt.fontWeight() >= QFont::Bold;
        bool italicsNow = fmt.fontItalic();
        bool underlineNow = fmt.fontUnderline();
        QColor backgroundNow(fmt.background().color());
        QColor foregroundNow(fmt.foreground().color());
        QFont fontNow(fmt.font());

        if (bold != boldNow ||
            italics != italicsNow ||
            underline != underlineNow ||
            background != backgroundNow ||
            foreground != foregroundNow ||
            fonted != fontNow)
        {
            if (opened)
            {
                text.append("</span>");
            }

            QString style("<span style='");
            if (backgroundNow != bg)
            {
                style.append(QString("background: %1; ").arg(backgroundNow.name()));
            }
            if (foregroundNow != fg)
            {
                style.append(QString("color: %1; ").arg(foregroundNow.name()));
            }
            if (fontNow.family() != font.family())
            {
                style.append(QString("font-family: %1; ").arg(fontNow.family()));
            }
            if (fontNow.pixelSize() != font.pixelSize())
            {
                style.append(QString("font-size: %1px; ").arg(fontNow.pixelSize()));
            }
            if (boldNow)
            {
                style.append("font-weight: bold; ");
            }
            if (italicsNow)
            {
                style.append("font-style: italics; ");
            }
            if (underlineNow)
            {
                style.append("font-decoration: underline; ");
            }
            style.append("'>");

            if (style.contains(":"))
            {
                text.append(style);
                opened = true;
            }

            bold = boldNow;
            italics = italicsNow;
            underline = underlineNow;
            background = backgroundNow;
            foreground = foregroundNow;
            fonted = fontNow;
        }

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

        if (cursor.atBlockEnd())
        {
            if (opened)
            {
                text.append("</span>");

                bold = false;
                italics = false;
                underline = false;
                background = bg;
                foreground = fg;
                fonted = font;

                opened = false;
            }

            text.append("\n");
        }
    }

    if (opened)
    {
        text.append("</span>");
    }

    return text;
}

void TextDocument::clear()
{
    QTextDocument::clear();

    m_cursor = new QTextCursor(this);
    m_cursor->setCharFormat(m_formatCurrent);
}

void TextDocument::appendLink(const QString &text, const QString &link, const QColor &fg, const QColor &bg)
{
    QTextCharFormat previousFormat(m_cursor->charFormat());

    QTextCharFormat format;
    format.setForeground(fg);
    format.setBackground(bg);
    format.setAnchor(true);
    format.setAnchorHref(link);
    m_cursor->mergeCharFormat(format);

    for (int n = 0; n < text.length(); n++)
    {
        QChar ch(text.at(n));

        if (CRLF.contains(ch))
        {
            newLine();
            continue;
        }

        m_cursor->insertText(ch);
    }

    m_cursor->setCharFormat(previousFormat);
}

void TextDocument::dump()
{
    qDebug() << "----------";
    QTextBlock currentBlock(begin());
    while (currentBlock.isValid())
    {
        TextBlockData *tbd = dynamic_cast<TextBlockData *>(currentBlock.userData());
        if (tbd)
        {
            qDebug() << "block" << currentBlock.blockNumber() << tbd->timestamp().toString() << currentBlock.text();
        }
        else
        {
            qDebug() << "block" << currentBlock.blockNumber() << "no timestamp" << currentBlock.text();
        }
        currentBlock = currentBlock.next();
    }
}

void TextDocument::process(const QByteArray &data)
{
    const QDateTime timePacket(QDateTime::currentDateTime());

    saveSelection();

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

            emit lineAdded(m_cursor->block());

            // TODO: gag/replace lines

            if (!m_isPrompt)
            {
                m_cursor->insertBlock();
                m_cursor->block().setUserData(new TextBlockData(timePacket));
            }

            continue;
        }

        m_text.append(ch);
    }

    restoreSelection();
}

void TextDocument::append(const QString &text, const QColor &fg, const QColor &bg)
{
    saveSelection();

    QTextCharFormat previousFormat(m_cursor->charFormat());

    QTextCharFormat format;
    format.setForeground(fg);
    format.setBackground(bg);
    m_cursor->mergeCharFormat(format);

    for (int n = 0; n < text.length(); n++)
    {
        QChar ch(text.at(n));

        if (CRLF.contains(ch) || m_isPrompt)
        {
            newLine();
            continue;
        }

        m_cursor->insertText(ch);
    }

    m_cursor->setCharFormat(previousFormat);

    restoreSelection();
}

void TextDocument::command(const QString &text)
{
    saveSelection();

    QTextCharFormat previousFormat(m_cursor->charFormat());

    m_cursor->mergeCharFormat(m_formatCommand);

    m_cursor->insertText(text);

    newLine();

    m_cursor->setCharFormat(previousFormat);

    restoreSelection();
}

void TextDocument::newLine()
{
    m_cursor->insertBlock();
    m_cursor->block().setUserData(new TextBlockData(QDateTime::currentDateTime()));

    m_isPrompt = false;
}

void TextDocument::selectAll()
{
    m_cursor->select(QTextCursor::Document);

    emit contentsChanged();
}

void TextDocument::selectNone()
{
    m_cursor->clearSelection();

    emit contentsChanged();
}

void TextDocument::select(int start, int stop)
{
    if (start == stop)
    {
        selectNone();
        return;
    }

    m_cursor->setPosition(start);
    m_cursor->setPosition(stop, QTextCursor::KeepAnchor);

    emit contentsChanged();
}

void TextDocument::processAnsi(int code)
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

inline QColor TextDocument::translateColor(const QString &name)
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

void TextDocument::saveSelection()
{
    m_selected = m_cursor->hasSelection();
    m_selectionStart = m_cursor->selectionStart();
    m_selectionEnd = m_cursor->selectionEnd();

    if (m_selected)
    {
        m_cursor->clearSelection();
        m_cursor->movePosition(QTextCursor::End);
    }
}

void TextDocument::restoreSelection()
{
    if (m_selected)
    {
        m_cursor->setPosition(m_selectionStart);
        m_cursor->setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    }
}
