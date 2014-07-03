/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  jkdoug@gmail.com

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


#ifndef CONSOLEDOCUMENT_H
#define CONSOLEDOCUMENT_H

#include <QList>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>

class ConsoleDocument : public QTextDocument
{
    Q_OBJECT
public:
    explicit ConsoleDocument(QObject *parent = 0);

    bool hasSelection() const { return m_selection.hasSelection(); }
    QTextCursor selection() const { return m_selection; }
    QTextCharFormat formatSelection() const { return m_formatSelection; }

    QString toPlainText(int start, int stop);
    QString toPlainText(QTextCursor cur = QTextCursor());

    QString toHtml(int start, int stop, const QColor &fg = QColor(), const QColor &bg = QColor(), const QFont &font = QFont());
    QString toHtml(QTextCursor cur = QTextCursor(), const QColor &fg = QColor(), const QColor &bg = QColor(), const QFont &font = QFont());

    void deleteBlock(const QTextBlock &block);
    void deleteLines(int count);
    void omit() { m_omit = true; }

public slots:
    void process(const QByteArray &data);
    void command(const QString &cmd);
    void error(const QString &msg);
    void warning(const QString &msg);
    void info(const QString &msg);
    void optionChanged(const QString &key, const QVariant &val);
    void select(int start, int stop);
    void selectAll();
    void selectNone();
    virtual void clear();

signals:
    void blockAdded(QTextBlock block);

private:
    void newLine();
    void processAnsi(int code);
    QColor translateColor(const QString &name);
    void appendText(const QTextCharFormat &fmt, const QString &text, bool newline = true);

    QTextCursor m_cursor;
    QTextCursor m_selection;

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
    bool m_omit;

    int m_gagLine;

    QTextCharFormat m_formatDefault;
    QTextCharFormat m_formatSelection;
    QTextCharFormat m_formatCommand;
    QTextCharFormat m_formatCurrent;
    QTextCharFormat m_formatError;
    QTextCharFormat m_formatWarning;
    QTextCharFormat m_formatInfo;
};

#endif // CONSOLEDOCUMENT_H
