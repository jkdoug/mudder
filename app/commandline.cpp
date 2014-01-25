/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "commandline.h"
#include "console.h"
#include <QFontMetrics>
#include <QKeySequence>
#include <QLayout>
#include <QRegularExpression>

CommandLine::CommandLine(QWidget *parent) :
    QPlainTextEdit(parent)
{
    m_completer = new QCompleter(this);
    m_completionModel = new QStringListModel(m_completer);

    m_completer->setModel(m_completionModel);
    m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setWrapAround(false);

    m_completer->setWidget(this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);

    connect(m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

void CommandLine::insertCompletion(const QString &completion)
{
    if (m_completer->widget() != this)
    {
        return;
    }

    int extra = completion.length() - m_completer->completionPrefix().length();

    QTextCursor tc(textCursor());
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));

    setTextCursor(tc);
}

void CommandLine::keyPressEvent(QKeyEvent *e)
{
    if (m_completer && m_completer->popup() && m_completer->popup()->isVisible())
    {
       switch (e->key())
       {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return;

       default:
           break;
       }
    }

    const bool isShortcut = e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_E;
    if (!m_completer || !isShortcut)
    {
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (e->modifiers().testFlag(Qt::ShiftModifier))
            {
                textCursor().insertBlock();
            }
            else if (e->modifiers().testFlag(Qt::ControlModifier))
            {
                console()->scrollToBottom();
            }
            else
            {
                send();
            }
            e->accept();
            return;

        case Qt::Key_Down:
            if (e->modifiers().testFlag(Qt::ControlModifier))
            {
                moveCursor(QTextCursor::Down);
            }
            else
            {
                historyDown();
                adjustHeight();
            }
            e->accept();
            return;

        case Qt::Key_Up:
            if (e->modifiers().testFlag(Qt::ControlModifier))
            {
                moveCursor(QTextCursor::Up);
            }
            else
            {
                historyUp();
                adjustHeight();
            }
            e->accept();
            return;

        case Qt::Key_Escape:
            if (e->modifiers() == 0)
            {
                console()->scrollToBottom();
                e->accept();
                return;
            }
            break;

        default:
            if (console()->processKey(QKeySequence(e->key() + e->modifiers())))
            {
                e->accept();
                return;
            }
            break;
        }

        updateCompletions();

        QPlainTextEdit::keyPressEvent(e);
        adjustHeight();
    }

    const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) ||
            e->modifiers().testFlag(Qt::ShiftModifier);
    if (!m_completer || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString endOfWord("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix(textUnderCursor());

    if (!isShortcut && (hasModifier || e->text().isEmpty() ||
        completionPrefix.length() < 3 || endOfWord.contains(e->text().right(1))))
    {
        m_completer->popup()->hide();
        return;
    }

    if (completionPrefix != m_completer->completionPrefix())
    {
        m_completer->setCompletionPrefix(completionPrefix);
        m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    }

    QRect cr(cursorRect());
    cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
                + m_completer->popup()->verticalScrollBar()->sizeHint().width());
    m_completer->complete(cr);
}

void CommandLine::focusInEvent(QFocusEvent *e)
{
    if (m_completer)
    {
        m_completer->setWidget(this);
    }

    QPlainTextEdit::focusInEvent(e);
}

QString CommandLine::textUnderCursor() const
{
    QTextCursor tc(textCursor());
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CommandLine::updateCompletions()
{
//    QStringList recentLines(console()->buffer()->lastLines(250));
//    QStringList goodWords;
//    foreach (QString line, recentLines)
//    {
//        QStringList words(line.split(QRegularExpression("\\b")));
//        foreach (QString word, words)
//        {
//            if (word.length() > 3)
//            {
//                goodWords.append(word);
//            }
//        }
//    }

//    goodWords.removeDuplicates();
//    goodWords.sort(Qt::CaseInsensitive);
//    m_completionModel->setStringList(goodWords);
}

void CommandLine::send()
{
    QString txt(toPlainText());
    QString prefix(console()->profile()->scriptPrefix());
    if (txt.startsWith(prefix))
    {
        txt = txt.remove(0, prefix.length());
        emit script(txt);
    }
    else
    {
        QString sep(console()->profile()->commandSeparator() + QString("\n"));
        QRegularExpression regex("[" + QRegularExpression::escape(sep) + "]");
        QStringList cmds(txt.split(regex, QString::SkipEmptyParts));
        foreach (const QString &cmd, cmds)
        {
            emit command(cmd);
        }
    }

    if (console()->profile()->clearCommandAfterSend())
    {
        clear();
    }
    else
    {
        selectAll();
    }

    txt = toPlainText();
    if (!txt.isEmpty())
    {
        m_historyPosition = 0;

        m_history.removeAll(txt);
        m_history.push_front(txt);
    }
    else
    {
        m_historyPosition = -1;
    }
}

void CommandLine::historyUp()
{
    if (m_history.isEmpty())
    {
        return;
    }

    QString txt(toPlainText());
    if (textCursor().selectedText().size() == txt.size() || txt.isEmpty())
    {
        if (!txt.isEmpty())
        {
            m_historyPosition++;
        }

        m_historyPosition = qBound(0, m_historyPosition, m_history.length() - 1);

        setPlainText(m_history.at(m_historyPosition));
        selectAll();
    }
}

void CommandLine::historyDown()
{
    if (m_history.isEmpty())
    {
        return;
    }

    QString txt(toPlainText());
    if (textCursor().selectedText().size() == txt.size() || txt.isEmpty())
    {
        m_historyPosition = qBound(0, --m_historyPosition, m_history.length() - 1);

        setPlainText(m_history.at(m_historyPosition));
        selectAll();
    }
}

void CommandLine::adjustHeight()
{
    int lineCount = qBound(1, document()->size().toSize().height(), 10);
    int fontHeight = QFontMetrics(font()).height();
    int baseHeight = fontHeight * lineCount;
    int desiredHeight = qMax(baseHeight + fontHeight, 31);

    if (height() != desiredHeight)
    {
        setMinimumHeight(desiredHeight);
        setMaximumHeight(desiredHeight);
    }
}
