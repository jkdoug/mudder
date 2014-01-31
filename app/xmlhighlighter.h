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

#ifndef XMLHIGHLIGHTER_H
#define XMLHIGHLIGHTER_H

#include "syntaxhighlighter.h"

class XmlHighlighter : public SyntaxHighlighter
{
    Q_OBJECT

public:
    explicit XmlHighlighter(QTextDocument *document = 0);

protected:
    void highlightBlock(const QString &text);

    void highlightComment(const QString &text);
    void highlightDTD(const QString &text);
    void highlightPI(const QString &text);
    void highlightQuotes(const QString &text);

private:
    enum BlockState
    {
        NoBlock = 0,
        InComment,
        InDTD,
        InPI,
        InElement
    };

    QRegularExpression m_piStart;
    QRegularExpression m_piEnd;
    QRegularExpression m_piName;
    QRegularExpression m_commentStart;
    QRegularExpression m_commentEnd;
    QRegularExpression m_dtdStart;
    QRegularExpression m_dtdEnd;
    QRegularExpression m_dtdName;
    QRegularExpression m_quoteStart;
    QRegularExpression m_quoteEnd;
    QRegularExpression m_openStart;
    QRegularExpression m_openEnd;
    QRegularExpression m_closeStart;
    QRegularExpression m_closeEnd;
    QRegularExpression m_attributeStart;
    QRegularExpression m_attributeEnd;
    QRegularExpression m_valueStart;
    QRegularExpression m_valueEnd;

    QTextCharFormat m_piFormat;
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_dtdFormat;
    QTextCharFormat m_dtdNameFormat;
    QTextCharFormat m_quoteFormat;
    QTextCharFormat m_tagFormat;
    QTextCharFormat m_attributeFormat;

//Enumeration for types of element, used for tracking what
//we're inside while highlighting over multiline blocks.
    enum xmlState {
        inNothing,     //Don't know if we'll need this or not.
        inProcInst,    //starting with <? and ending with ?>
        inDoctypeDecl, //starting with <!DOCTYPE and ending with >
        inOpenTag,     //starting with < + xmlName and ending with /?>
        inOpenTagName, //after < and before whitespace. Implies inOpenTag.
        inAttribute,   //if inOpenTag, starting with /s*xmlName/s*=/s*" and ending with ".
        inAttName,     //after < + xmlName + whitespace, and before =". Implies inOpenTag.
        inAttVal,      //after =" and before ". May also use single quotes. Implies inOpenTag.
        inCloseTag,    //starting with </ and ending with >.
        inCloseTagName,//after </ and before >. Implies inCloseTag.
        inComment      //starting with <!-- and ending with -->. Overrides all others.
    };
};


#endif
