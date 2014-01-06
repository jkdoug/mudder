#ifndef LUAEDIT_H
#define LUAEDIT_H

#include <QChar>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPair>
#include <QPlainTextEdit>
#include <QPoint>
#include <QRect>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QTextBlock>
#include <QWidget>

class LuaHighlighter;

class LuaEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit LuaEdit(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateCursorPosition();
    void updateLineNumberArea(const QRect &rect, int dy);
    void updateLineNumberAreaWidth(int newBlockCount);

private:
    bool matchLeftBrace(QTextBlock currentBlock, int index, QChar brace, int numLeft = 0);
    bool matchRightBrace(QTextBlock currentBlock, int index, QChar brace, int numRight = 0);
    void createBraceSelection(int pos);

    LuaHighlighter *m_highlighter;
    QWidget *m_lineNumberArea;
    QPair<int, int> m_countCache;
};

#endif // LUAEDIT_H
