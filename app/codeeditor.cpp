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


#include "codeeditor.h"
#include "options.h"
#include <QtDebug>
#include <QPainter>
#include <QSyntaxHighlighter>

struct BraceInfo
{
    QChar character;
    bool left;
    int position;
};

class CodeBlockData : public QTextBlockUserData
{
public:
    explicit CodeBlockData() {}

    QVector<BraceInfo *> braces() { return m_braces; }

    void insert(BraceInfo *info)
    {
        int pos = 0;
        while (pos < m_braces.size() && info->position > m_braces.at(pos)->position)
        {
            pos++;
        }

        m_braces.insert(pos, info);
    }

private:
    QVector<BraceInfo *> m_braces;
};

struct CodeEditorData
{
    CodeEditorData() :
        lineNumberArea(0),
        syntaxHighlighter(0)
    {
        countCache.first = -1;
        countCache.second = -1;
    }

    QWidget * lineNumberArea;
    QSyntaxHighlighter * syntaxHighlighter;
    QPair<int, int> countCache;
};


class CodeEditor;

class LineNumberAreaPrivate : public QWidget
{
public:
    LineNumberAreaPrivate(CodeEditor *editor) :
        QWidget(editor)
    {
        textEditor = editor;
    }

    QSize sizeHint() const
    {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *textEditor;
};


CodeEditor::CodeEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    d = new CodeEditorData;

    d->lineNumberArea = new LineNumberAreaPrivate(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursorPosition()));

    setFont(Options::editorFont());

    updateLineNumberAreaWidth(0);
}

CodeEditor::~CodeEditor()
{
    delete d;
}

QSyntaxHighlighter * CodeEditor::syntaxHighlighter() const
{
    return d->syntaxHighlighter;
}

void CodeEditor::setSyntaxHighlighter(QSyntaxHighlighter *highlighter)
{
    d->syntaxHighlighter = highlighter;
    d->syntaxHighlighter->setDocument(document());
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(d->lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block(firstVisibleBlock());
    int blockNumber = block.blockNumber();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            painter.setPen(Qt::black);
            painter.drawText(0, top, d->lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateCursorPosition()
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

    CodeBlockData *data = static_cast<CodeBlockData *>(textCursor().block().userData());
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

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount)

    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
    {
        d->lineNumberArea->scroll(0, dy);
    }
    else if (d->countCache.first != blockCount() || d->countCache.second != textCursor().block().lineCount())
    {
        d->lineNumberArea->update(0, rect.y(), d->lineNumberArea->width(), rect.height());

        d->countCache.first = blockCount();
        d->countCache.second = textCursor().block().lineCount();
    }

    if (rect.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth(0);
    }
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr(contentsRect());
    d->lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightWords(const QStringList &words, const QBrush &brush)
{
    QTextCursor cursor(textCursor());
    cursor.movePosition(QTextCursor::Start);
    setTextCursor(cursor);

    foreach (const QString &word, words)
    {
        while (find(word))
        {
            QTextCursor find_cursor(document()->find(word, cursor));
            QTextCharFormat format(find_cursor.charFormat());
            format.setBackground(brush);
            find_cursor.mergeCharFormat(format);
        }

        cursor.movePosition(QTextCursor::Start);
        setTextCursor(cursor);
    }

    cursor.movePosition(QTextCursor::Start);
    setTextCursor(cursor);
}

void CodeEditor::removeWordHighlighting()
{
    QString tmp(document()->toPlainText());
    document()->clear();
    document()->setPlainText(tmp);
}

inline QChar CodeEditor::leftBrace(QChar right)
{
    switch (right.toLatin1())
    {
    case ')':
        return '(';

    case ']':
        return '[';

    case '}':
        return '{';
    }

    Q_ASSERT(false);
    return QChar();
}

inline QChar CodeEditor::rightBrace(QChar left)
{
    switch (left.toLatin1())
    {
    case '(':
        return ')';

    case '[':
        return ']';

    case '{':
        return '}';
    }

    Q_ASSERT(false);
    return QChar();
}

bool CodeEditor::matchLeftBrace(QTextBlock currentBlock, int index, QChar brace, int numLeft)
{
    CodeBlockData *data = static_cast<CodeBlockData *>(currentBlock.userData());
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

        if (info->character == rightBrace(brace))
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

bool CodeEditor::matchRightBrace(QTextBlock currentBlock, int index, QChar brace, int numRight)
{
    CodeBlockData *data = static_cast<CodeBlockData *>(currentBlock.userData());
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

        if (info->character == leftBrace(brace))
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

void CodeEditor::createBraceSelection(int pos)
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

