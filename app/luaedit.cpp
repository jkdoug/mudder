#include "luaedit.h"
#include "luahighlighter.h"
#include "lualinenumberarea.h"

#include <QDebug>
#include <QPainter>
#include <QSettings>
#include <QString>
#include <QTextBlock>

LuaEdit::LuaEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
    m_highlighter = new LuaHighlighter(document());
    m_lineNumberArea = new LuaLineNumberArea(this);

    m_countCache.first = -1;
    m_countCache.second = -1;

    QSettings settings;

    bool antiAlias = settings.value("GlobalFontAntialias", true).toBool();
    QString fontName(settings.value("GlobalFontName", "Consolas").toString());
    int fontSize = settings.value("GlobalFontSize", 9).toInt();

    QFont font(fontName, fontSize);
    font.setStyleHint(QFont::TypeWriter);
    font.setStyleStrategy(antiAlias?QFont::PreferAntialias:QFont::NoAntialias);
    setFont(font);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursorPosition()));

    updateLineNumberAreaWidth(0);
    updateCursorPosition();
}

void LuaEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block(firstVisibleBlock());
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number(QString::number(blockNumber + 1));
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}

int LuaEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        digits++;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void LuaEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr(contentsRect());
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void LuaEdit::updateCursorPosition()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(QColor(Qt::yellow).lighter(190));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        selection.cursor = textCursor();
        selection.cursor.clearSelection();

        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);

    LuaBlockData *data = static_cast<LuaBlockData *>(textCursor().block().userData());
    if (data)
    {
        QVector<BraceInfo *> infos(data->braces());
        int pos = textCursor().block().position();

        for (int index = 0; index < infos.size(); index++)
        {
            BraceInfo *info = infos.at(index);

            int currentPos = textCursor().position() - textCursor().block().position();

            if (info->position == currentPos - 1 && info->left)
            {
                if (matchLeftBrace(textCursor().block(), index + 1, info->character))
                {
                    createBraceSelection(pos + info->position);
                }
            }
            else if (info->position == currentPos - 1 && !info->left)
            {
                if (matchRightBrace(textCursor().block(), index - 1, info->character))
                {
                    createBraceSelection(pos + info->position);
                }
            }
        }
    }
}

void LuaEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
    {
        m_lineNumberArea->scroll(0, dy);
    }
    else if (m_countCache.first != blockCount() || m_countCache.second != textCursor().block().lineCount())
    {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

        m_countCache.first = blockCount();
        m_countCache.second = textCursor().block().lineCount();
    }

    if (rect.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth(0);
    }
}

void LuaEdit::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount)

    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

bool LuaEdit::matchLeftBrace(QTextBlock currentBlock, int index, QChar brace, int numLeft)
{
    LuaBlockData *data = static_cast<LuaBlockData *>(currentBlock.userData());
    QVector<BraceInfo *> infos(data->braces());

    int blockPos = currentBlock.position();
    for (int infoPos = index; infoPos < infos.size(); infoPos++)
    {
        BraceInfo *info = infos.at(infoPos);

        if (info->character == brace)
        {
            numLeft++;
            continue;
        }

        if (info->character == LuaHighlighter::rightBrace(brace))
        {
            if (numLeft == 0)
            {
                createBraceSelection(blockPos + info->position);
                return true;
            }
            else
            {
                numLeft--;
            }
        }
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())\
    {
        return matchLeftBrace(currentBlock, 0, brace, numLeft);
    }

    return false;
}

bool LuaEdit::matchRightBrace(QTextBlock currentBlock, int index, QChar brace, int numRight)
{
    LuaBlockData *data = static_cast<LuaBlockData *>(currentBlock.userData());
    QVector<BraceInfo *> infos(data->braces());

    int blockPos = currentBlock.position();
    for (int infoPos = index; infoPos > -1 && infos.size() > 0; infoPos--)
    {
        BraceInfo *info = infos.at(infoPos);

        if (info->character == brace)
        {
            numRight++;
            continue;
        }

        if (info->character == LuaHighlighter::leftBrace(brace))
        {
            if (numRight == 0)
            {
                createBraceSelection(blockPos + info->position);
                return true;
            }
            else
            {
                numRight--;
            }
        }
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())\
    {
        return matchRightBrace(currentBlock, 0, brace, numRight);
    }

    return false;
}

void LuaEdit::createBraceSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections(extraSelections());

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format(selection.format);
    format.setBackground(QColor(Qt::green).lighter(160));
    format.setForeground(QColor(Qt::red));
    selection.format = format;

    QTextCursor cursor(textCursor());
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
}
