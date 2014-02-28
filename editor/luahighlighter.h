#ifndef LUAHIGHLIGHTER_H
#define LUAHIGHLIGHTER_H

#include "editor_global.h"
#include "syntaxhighlighter.h"

class EDITORSHARED_EXPORT LuaHighlighter : public SyntaxHighlighter
{
    Q_OBJECT
public:
    explicit LuaHighlighter(QTextDocument *document = 0);

    virtual QRegularExpression leftBraceExpression() { return QRegularExpression("([\\[{\\(])"); }
    virtual QRegularExpression rightBraceExpression() { return QRegularExpression("([\\]}\\)])"); }

protected:
    void highlightBlock(const QString &text);

private:
    QRegularExpression m_commentStart;
    QRegularExpression m_commentEnd;

    QTextCharFormat m_singleLineCommentFormat;
    QTextCharFormat m_multiLineCommentFormat;
    QTextCharFormat m_quotationFormat;
    QTextCharFormat m_functionFormat;
};

#endif // LUAHIGHLIGHTER_H
