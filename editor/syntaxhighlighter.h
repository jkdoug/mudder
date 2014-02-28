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


#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include "editor_global.h"
#include <QChar>
#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextBlockUserData>
#include <QTextCharFormat>
#include <QVector>

class EDITORSHARED_EXPORT SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument *document = 0);

    virtual QRegularExpression leftBraceExpression() { return QRegularExpression(); }
    virtual QRegularExpression rightBraceExpression() { return QRegularExpression(); }
    virtual QChar leftBrace(QChar right);
    virtual QChar rightBrace(QChar left);

    void addKeywords(const QStringList &words, bool boundary = true);

protected:
    void highlightBlock(const QString &text);

    struct BraceInfo
    {
        QChar character;
        bool left;
        int position;
    };

    class BlockData : public QTextBlockUserData
    {
    public:
        explicit BlockData() {}

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

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> m_highlightingRules;

    QTextCharFormat m_keywordFormat;
};

#endif // SYNTAXHIGHLIGHTER_H
