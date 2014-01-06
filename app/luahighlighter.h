#ifndef LUAHIGHLIGHTER_H
#define LUAHIGHLIGHTER_H

#include <QChar>
#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextBlockUserData>
#include <QTextCharFormat>
#include <QVector>

struct BraceInfo
{
    QChar character;
    bool left;
    int position;
};

class LuaBlockData : public QTextBlockUserData
{
public:
    explicit LuaBlockData();

    QVector<BraceInfo *> braces() { return m_braces; }
    void insert(BraceInfo *info);

private:
    QVector<BraceInfo *> m_braces;
};

class LuaHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit LuaHighlighter(QTextDocument *document = 0);

    static QRegularExpression leftBraceExpression() { return QRegularExpression("([\\[{\\(])"); }
    static QRegularExpression rightBraceExpression() { return QRegularExpression("([\\]}\\)])"); }
    static QChar leftBrace(QChar right);
    static QChar rightBrace(QChar left);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> m_highlightingRules;

    QRegularExpression m_commentStartExpression;
    QRegularExpression m_commentEndExpression;

    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_singleLineCommentFormat;
    QTextCharFormat m_multiLineCommentFormat;
    QTextCharFormat m_quotationFormat;
    QTextCharFormat m_functionFormat;
};

#endif // LUAHIGHLIGHTER_H
