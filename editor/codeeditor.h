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


#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include "editor_global.h"
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>

struct CodeEditorData;

class EDITORSHARED_EXPORT CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = 0);
    virtual ~CodeEditor();

    QSyntaxHighlighter * syntaxHighlighter() const;
    void setSyntaxHighlighter(QSyntaxHighlighter *highlighter);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void removeWordHighlighting();

public slots:
    void updateCursorPosition();

    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

    void highlightWords(const QStringList &words, const QBrush &brush);

private slots:
    void handleSettingsChange(const QString &key, const QVariant &val);

protected:
    void resizeEvent(QResizeEvent *e);

private:
    QChar leftBrace(QChar right);
    QChar rightBrace(QChar left);

    bool matchLeftBrace(const QTextBlock &currentBlock, int index, QChar brace, int numLeft = 0);
    bool matchRightBrace(const QTextBlock &currentBlock, int index, QChar brace, int numRight = 0);
    void createBraceSelection(int pos);

    CodeEditorData *d;
};

#endif // CODEEDITOR_H
