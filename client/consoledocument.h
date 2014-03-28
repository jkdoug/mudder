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

    QString toPlainText(int start, int stop);
    QString toPlainText(QTextCursor cur = QTextCursor());

    void deleteBlock(int num);

public slots:
    void process(const QByteArray &data);
    void command(const QString &cmd);

signals:
    void blockAdded(QTextBlock block, bool prompt);

private:
    void newLine();
    void processAnsi(int code);
    QColor translateColor(const QString &name);

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

    int m_gagLine;

    QTextCharFormat m_formatDefault;
    QTextCharFormat m_formatCommand;
    QTextCharFormat m_formatCurrent;
};

#endif // CONSOLEDOCUMENT_H
