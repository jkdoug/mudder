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
    void highlightSubBlock(const QString &text, const int startIndex, const int currState);

private:
    QRegExp xmlProcInstStartExpression;
    QRegExp xmlProcInstEndExpression;
    QRegExp xmlCommentStartExpression;
    QRegExp xmlCommentEndExpression;
    QRegExp xmlDoctypeStartExpression;
    QRegExp xmlDoctypeEndExpression;

    QRegExp xmlOpenTagStartExpression;
    QRegExp xmlOpenTagEndExpression;
    QRegExp xmlCloseTagStartExpression;
    QRegExp xmlCloseTagEndExpression;
    QRegExp xmlAttributeStartExpression;
    QRegExp xmlAttributeEndExpression;
    QRegExp xmlAttValStartExpression;
    QRegExp xmlAttValEndExpression;

    QRegExp xmlAttValExpression;


    QTextCharFormat xmlProcInstFormat;
    QTextCharFormat xmlDoctypeFormat;
    QTextCharFormat xmlCommentFormat;
    QTextCharFormat xmlTagFormat;
    QTextCharFormat xmlEntityFormat;
    QTextCharFormat xmlAttributeFormat;
    QTextCharFormat xmlAttValFormat;

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
