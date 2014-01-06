#include "luahighlighter.h"

#include <QDebug>

LuaBlockData::LuaBlockData()
{
}

void LuaBlockData::insert(BraceInfo *info)
{
    int pos = 0;
    while (pos < m_braces.size() && info->position > m_braces.at(pos)->position)
    {
        pos++;
    }

    m_braces.insert(pos, info);
}

LuaHighlighter::LuaHighlighter(QTextDocument *document) :
    QSyntaxHighlighter(document)
{
    HighlightingRule rule;

    m_keywordFormat.setForeground(Qt::darkBlue);
    m_keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "and" << "break" << "do" << "else" << "elseif"
        << "end" << "false" << "for" << "function" << "global" << "goto "
        << "if" << "in" << "local" << "nil" << "not" << "or" << "repeat"
        << "return" << "then" << "true" << "until" << "while";
    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegularExpression(tr("\\b%1\\b").arg(pattern));
        rule.format = m_keywordFormat;
        m_highlightingRules.append(rule);
    }

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

    m_commentStartExpression = QRegularExpression("--\\[\\[");
    m_commentEndExpression = QRegularExpression("--\\]\\]");
}

QChar LuaHighlighter::leftBrace(QChar right)
{
    switch (right.toLatin1())
    {
    case ')':
        return '(';

    case ']':
        return '[';

    case '}':
        return '{';
    }

    Q_ASSERT(false);
    return QChar();
}

QChar LuaHighlighter::rightBrace(QChar left)
{
    switch (left.toLatin1())
    {
    case '(':
        return ')';

    case '[':
        return ']';

    case '{':
        return '}';
    }

    Q_ASSERT(false);
    return QChar();
}


void LuaHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, m_highlightingRules)
    {
        QRegularExpression expression(rule.pattern);
        QRegularExpressionMatch m(expression.match(text));
        while (m.hasMatch())
        {
            int start = m.capturedStart();
            int length = m.capturedLength();
            setFormat(start, length, rule.format);
            m = expression.match(text, start + length);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
    {
        startIndex = m_commentStartExpression.match(text).capturedStart();
    }

    while (startIndex >= 0)
    {
        QRegularExpressionMatch m(m_commentEndExpression.match(text, startIndex));
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
        startIndex = m_commentStartExpression.match(text, startIndex + commentLength).capturedStart();
    }


    LuaBlockData *data = new LuaBlockData;

    QRegularExpressionMatch matchLeft(leftBraceExpression().match(text));
    while (matchLeft.hasMatch())
    {
        Q_ASSERT(matchLeft.capturedLength() == 1);

        int pos = matchLeft.capturedStart();

        BraceInfo *info = new BraceInfo;
        info->character = text.at(pos);
        info->left = true;
        info->position = pos;

//        qDebug() << "match left" << info->character << info->position;

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

//        qDebug() << "match right" << info->character << info->position;

        data->insert(info);

        matchRight = rightBraceExpression().match(text, pos + 1);
    }

    setCurrentBlockUserData(data);
}
