/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  jkdoug@gmail.com

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
#include "coresettings.h"
#include <QtDebug>
#include <QPainter>
#include <QSyntaxHighlighter>


class CodeEditor;

class LineNumberAreaPrivate : public QWidget
{
public:
    LineNumberAreaPrivate(CodeEditor *editor) :
        QWidget(editor)
    {
        m_textEditor = editor;
    }

    QSize sizeHint() const
    {
        return QSize(m_textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        m_textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *m_textEditor;
};


CodeEditor::CodeEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    m_countCache.first = -1;
    m_countCache.second = -1;

    m_lineNumberArea = new LineNumberAreaPrivate(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursorPosition()));

    setFont(SETTINGS->value("Editor/Font", QFont("Consolas", 10)).value<QFont>());
    connect(SETTINGS, SIGNAL(valueChanged(QString,QVariant)), SLOT(handleSettingsChange(QString,QVariant)));

    updateLineNumberAreaWidth(0);

    setTabStopWidth(fontMetrics().averageCharWidth() * 2);
}

CodeEditor::~CodeEditor()
{
}

void CodeEditor::setSyntaxHighlighter(QSyntaxHighlighter *highlighter)
{
    m_syntaxHighlighter = highlighter;
    m_syntaxHighlighter->setDocument(document());
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
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
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumberArea->width(), fontMetrics().height(),
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

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr(contentsRect());
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
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

void CodeEditor::handleSettingsChange(const QString &key, const QVariant &val)
{
    if (key.compare("Editor/Font") == 0)
    {
        setFont(val.value<QFont>());
        setTabStopWidth(fontMetrics().averageCharWidth() * 2);
        update();
    }
}

