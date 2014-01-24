#ifndef LUAHIGHLIGHTER_H
#define LUAHIGHLIGHTER_H

#include "syntaxhighlighter.h"

class LuaHighlighter : public SyntaxHighlighter
{
    Q_OBJECT
public:
    explicit LuaHighlighter(QTextDocument *document = 0);

    virtual QRegularExpression leftBraceExpression() { return QRegularExpression("([\\[{\\(])"); }
    virtual QRegularExpression rightBraceExpression() { return QRegularExpression("([\\]}\\)])"); }

protected:
    void highlightBlock(const QString &text);

private:
    QRegularExpression m_commentStartExpression;
    QRegularExpression m_commentEndExpression;

    QTextCharFormat m_singleLineCommentFormat;
    QTextCharFormat m_multiLineCommentFormat;
    QTextCharFormat m_quotationFormat;
    QTextCharFormat m_functionFormat;
};

#endif // LUAHIGHLIGHTER_H
