#include "luahighlighter.h"

#include <QDebug>

LuaHighlighter::LuaHighlighter(QTextDocument *document) :
    SyntaxHighlighter(document)
{
    HighlightingRule rule;

    m_keywordFormat.setForeground(Qt::darkBlue);
    m_keywordFormat.setFontWeight(QFont::Bold);

    addKeywords(QStringList() << "and" << "break" << "do" << "else" << "elseif"
        << "end" << "false" << "for" << "function" << "global" << "goto "
        << "if" << "in" << "local" << "nil" << "not" << "or" << "repeat"
        << "return" << "then" << "true" << "until" << "while");

    m_quotationFormat.setForeground(Qt::magenta);
    rule.pattern = QRegularExpression("([\"\']).*?\\1");
    rule.format = m_quotationFormat;
    m_highlightingRules.append(rule);

    m_functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("\\b(?:[A-Za-z0-9_]+[\\.:])*[A-Za-z0-9_]+(?=\\()");
    rule.format = m_functionFormat;
    m_highlightingRules.append(rule);

    m_singleLineCommentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("--[^\n]*");
    rule.format = m_singleLineCommentFormat;
    m_highlightingRules.append(rule);

    m_multiLineCommentFormat.setForeground(Qt::gray);

    m_commentStart = QRegularExpression("--\\[\\[");
    m_commentEnd = QRegularExpression("--\\]\\]");
}

void LuaHighlighter::highlightBlock(const QString &text)
{
    SyntaxHighlighter::highlightBlock(text);

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
    {
        startIndex = m_commentStart.match(text).capturedStart();
    }

    while (startIndex >= 0)
    {
        QRegularExpressionMatch m(m_commentEnd.match(text, startIndex));
        int endIndex = m.capturedEnd();
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + m.capturedLength();
        }
        setFormat(startIndex, commentLength, m_multiLineCommentFormat);
        startIndex = m_commentStart.match(text, startIndex + commentLength).capturedStart();
    }
}
