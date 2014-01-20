#ifndef TEXTDOCUMENT_H
#define TEXTDOCUMENT_H

#include <QByteArray>
#include <QColor>
#include <QFont>
#include <QList>
#include <QSizeF>
#include <QString>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>

class TextDocument : public QTextDocument
{
    Q_OBJECT
public:
    explicit TextDocument(QObject *parent = 0);

    void setFont(const QFont &font);
    void setCommandColor(const QColor &fg, const QColor &bg = QColor());

    bool hasSelection() const { return m_cursor->hasSelection(); }
    QTextCursor *cursor() const { return m_cursor; }

    QString toPlainText(int start, int stop);
    QString toPlainText(const QTextCursor &cur = QTextCursor());
    QString toHtml(int start, int stop, const QColor &fg = QColor(), const QColor &bg = QColor(), const QFont &font = QFont());
    QString toHtml(const QTextCursor &cur = QTextCursor(), const QColor &fg = QColor(), const QColor &bg = QColor(), const QFont &font = QFont());

    void appendLink(const QString &text, const QString &link, const QColor &fg = QColor(), const QColor &bg = QColor(), const QString &tip = QString());

    void dump();

signals:
    void lineAdded(const QTextBlock &block);

public slots:
    void process(const QByteArray &data);
    void append(const QString &text, const QColor &fg = QColor(), const QColor &bg = QColor());
    void command(const QString &text);

    void newLine();

    virtual void clear();

    void selectAll();
    void selectNone();
    void select(int start, int stop);

private:
    void processAnsi(int code);
    QColor translateColor(const QString &name);

    void saveSelection();
    void restoreSelection();

    bool m_selected;
    int m_selectionStart;
    int m_selectionEnd;

    QTextCursor *m_cursor;

    QString m_text;
    QString m_input;
    QString m_ansiCode;
    QList<int> m_codes;

    bool m_gotESC;
    bool m_gotHeader;

    bool m_waitHighColorMode;
    bool m_fgHighColorMode;
    bool m_bgHighColorMode;
    bool m_isHighColorMode;
    bool m_isPrompt;

    QTextCharFormat m_formatDefault;
    QTextCharFormat m_formatCommand;
    QTextCharFormat m_formatCurrent;
};

#endif // TEXTDOCUMENT_H
