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


#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *document) :
    QSyntaxHighlighter(document)
{
}

QChar SyntaxHighlighter::leftBrace(QChar right)
{
    switch (right.toLatin1())
    {
    case ')':
        return '(';

    case ']':
        return '[';

    case '}':
        return '{';

    case '>':
        return '<';
    }

    Q_ASSERT(false);
    return QChar();
}

QChar SyntaxHighlighter::rightBrace(QChar left)
{
    switch (left.toLatin1())
    {
    case '(':
        return ')';

    case '[':
        return ']';

    case '{':
        return '}';

    case '<':
        return '>';
    }

    Q_ASSERT(false);
    return QChar();
}

void SyntaxHighlighter::addKeywords(const QStringList &words, bool boundary)
{
    HighlightingRule rule;
    foreach (const QString &word, words)
    {
        if (boundary)
        {
            rule.pattern = QRegularExpression(QString("\\b(%1)\\b").arg(word));
        }
        else
        {
            rule.pattern = QRegularExpression(QString("(%1)").arg(word));
        }

        rule.format = m_keywordFormat;
        m_highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, m_highlightingRules)
    {
        QRegularExpression expression(rule.pattern);
        QRegularExpressionMatch m(expression.match(text));
        while (m.hasMatch())
        {
            int start = m.capturedStart(1);
            int length = m.capturedLength(1);
            setFormat(start, length, rule.format);
            m = expression.match(text, start + length);
        }
    }

    BlockData *data = new BlockData;

    if (!leftBraceExpression().pattern().isEmpty())
    {
        QRegularExpressionMatch matchLeft(leftBraceExpression().match(text));
        while (matchLeft.hasMatch())
        {
            Q_ASSERT(matchLeft.capturedLength() == 1);

            int pos = matchLeft.capturedStart();

            BraceInfo *info = new BraceInfo;
            info->character = text.at(pos);
            info->left = true;
            info->position = pos;

            data->insert(info);

            matchLeft = leftBraceExpression().match(text, pos + 1);
        }

        QRegularExpressionMatch matchRight(rightBraceExpression().match(text));
        while (matchRight.hasMatch())
        {
            Q_ASSERT(matchRight.capturedLength() == 1);

            int pos = matchRight.capturedStart();

            BraceInfo *info = new BraceInfo;
            info->character = text.at(pos);
            info->left = false;
            info->position = pos;

            data->insert(info);

            matchRight = rightBraceExpression().match(text, pos + 1);
        }
    }

    setCurrentBlockUserData(data);
}
