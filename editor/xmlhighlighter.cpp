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


#include "xmlhighlighter.h"
#include <QtDebug>

XmlHighlighter::XmlHighlighter(QTextDocument *document) :
    SyntaxHighlighter(document)
{
    m_piStart.setPattern("<\\?");
    m_piEnd.setPattern("\\?>");
    m_piName.setPattern("<\\?([xX][mM][lL])");
    m_piFormat.setForeground(Qt::darkMagenta);
    m_piFormat.setFontWeight(QFont::Bold);

    m_commentStart = QRegularExpression("\\<!\\-\\-");
    m_commentEnd = QRegularExpression("\\-\\-\\>");
    m_commentFormat.setForeground(Qt::darkGreen);

    m_dtdStart.setPattern("<![^-]*");
    m_dtdEnd.setPattern(">");
    m_dtdName.setPattern("<!(DOCTYPE|ATTLIST|ENTITY|ELEMENT|NOTATION)");
    m_dtdFormat.setForeground(Qt::darkBlue);
    m_dtdFormat.setBackground(QColor(0xEF, 0xEF, 0xFF));
    m_dtdNameFormat = m_dtdFormat;
    m_dtdNameFormat.setFontWeight(QFont::Bold);

    const QString nameStartCharList = ":A-Z_a-z";
    const QString nameCharList = nameStartCharList + "\\-\\.0-9";
    const QString nameStart = "[" + nameStartCharList + "]";
    const QString nameChar = "[" + nameCharList + "]";
    const QString xmlName = nameStart + "(" + nameChar + ")*";

    m_quoteFormat.setForeground(Qt::darkMagenta);
    m_tagFormat.setForeground(Qt::darkBlue);
    m_attributeFormat.setForeground(Qt::darkCyan);
    m_entityFormat.setForeground(Qt::darkRed);
    m_entityFormat.setBackground(QColor("bisque"));

    HighlightingRule rule;
    rule.pattern.setPattern("(&" + xmlName + ";)");
    rule.format = m_entityFormat;
    m_highlightingRules.append(rule);

    rule.pattern.setPattern("(([\"\']).*?\\2)");
    rule.format = m_quoteFormat;
    m_highlightingRules.append(rule);

    rule.pattern.setPattern("<(" + xmlName + ")\\b");
    rule.format = m_tagFormat;
    m_highlightingRules.append(rule);

    rule.pattern.setPattern("</(" + xmlName + ")>");
    rule.format = m_tagFormat;
    m_highlightingRules.append(rule);

    rule.pattern.setPattern("(" + xmlName + ")\\s*\\=");
    rule.format = m_attributeFormat;
    m_highlightingRules.append(rule);
}

void XmlHighlighter::highlightBlock(const QString &text)
{
    setCurrentBlockState(NoBlock);

    highlightPI(text);
    highlightDTD(text);

    SyntaxHighlighter::highlightBlock(text);

    highlightComment(text);
}

void XmlHighlighter::highlightComment(const QString &text)
{
    int commentStart = 0;
    if (previousBlockState() != InComment)
    {
        commentStart = m_commentStart.match(text).capturedStart();
    }

    while (commentStart >= 0)
    {
        int commentEnd = m_commentEnd.match(text, commentStart).capturedEnd();
        if (commentEnd == -1)
        {
            setCurrentBlockState(InComment);
            commentEnd = text.length();
        }
        setFormat(commentStart, commentEnd - commentStart, m_commentFormat);
        commentStart = m_commentStart.match(text, commentEnd).capturedStart();
    }
}

void XmlHighlighter::highlightDTD(const QString &text)
{
    int dtdStart = 0;
    if (previousBlockState() != InDTD)
    {
        dtdStart = m_dtdStart.match(text).capturedStart();
    }

    while (dtdStart >= 0)
    {
        int dtdEnd = m_dtdEnd.match(text, dtdStart).capturedEnd();
        if (dtdEnd == -1)
        {
            setCurrentBlockState(InDTD);
            dtdEnd = text.length();
        }
        setFormat(dtdStart, dtdEnd - dtdStart, m_dtdFormat);

        QRegularExpressionMatch m(m_dtdName.match(text, dtdStart));
        int dtdNameStart = m.capturedStart(1);
        if (dtdNameStart > 0)
        {
            setFormat(dtdNameStart, m.capturedLength(1), m_dtdNameFormat);
        }

        dtdStart = m_dtdStart.match(text, dtdEnd).capturedStart();
    }
}

void XmlHighlighter::highlightPI(const QString &text)
{
    int piStart = 0;
    if (previousBlockState() != InPI)
    {
        piStart = m_piStart.match(text).capturedStart();
    }

    while (piStart >= 0)
    {
        int piEnd = m_piEnd.match(text, piStart).capturedEnd();
        if (piEnd == -1)
        {
            setCurrentBlockState(InPI);
            piEnd = text.length();
        }
        setFormat(piStart, piEnd - piStart, m_piFormat);

        QRegularExpressionMatch m(m_piName.match(text, piStart));
        int piNameStart = m.capturedStart(1);
        if (piNameStart > 0)
        {
            setFormat(piNameStart, m.capturedLength(1), m_tagFormat);
        }

        piStart = m_piStart.match(text, piEnd).capturedStart();
    }
}
