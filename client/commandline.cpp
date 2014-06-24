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


#include "commandline.h"
#include "coresettings.h"
#include <QAbstractItemView>
#include <QFont>
#include <QRegularExpression>
#include <QScrollBar>

CommandLine::CommandLine(QWidget *parent) :
    QPlainTextEdit(parent),
    m_historyPosition(-1),
    m_escapeClears(true),
    m_clearCommandLine(false),
    m_echoOn(true),
    m_accelerated(false),
    m_scriptPrefix("/"),
    m_commandSeparator(";")
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

    connect(m_completer, SIGNAL(activated(QString)), SLOT(insertCompletion(QString)));
    connect(this, SIGNAL(command(QString)), SLOT(addToHistory(QString)));

    updateCompletions();
}

void CommandLine::optionChanged(const QString &key, const QVariant &val)
{
    if (key == "inputFont")
    {
        setFont(val.value<QFont>());
    }
    else if (key == "escapeClearsCommand")
    {
        m_escapeClears = val.toBool();
    }
    else if (key == "clearCommandLine")
    {
        m_clearCommandLine = val.toBool();
    }
    else if (key == "scriptPrefix")
    {
        m_scriptPrefix = val.toString();
    }
    else if (key == "commandSeparator")
    {
        m_commandSeparator = val.toString();
    }
}

void CommandLine::echoToggled(bool flag)
{
    m_echoOn = flag;
}

void CommandLine::keyPressEvent(QKeyEvent *e)
{
    if (m_completer->popup()->isVisible())
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

    const bool isShortcut = e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_Space;
    if (!isShortcut)
    {
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (e->modifiers().testFlag(Qt::ShiftModifier))
            {
                textCursor().insertBlock();
                adjustHeight();
            }
            else
            {
                processCommand(toPlainText());
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
            if (m_escapeClears && e->modifiers() == 0)
            {
                clear();
                e->accept();
                return;
            }
            break;

        default:
            emit accelerator(QKeySequence(e->key() + e->modifiers()));

            if (m_accelerated)
            {
                m_accelerated = false;
                e->accept();
                return;
            }
            break;
        }

        QPlainTextEdit::keyPressEvent(e);

        adjustHeight();
    }

    const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) || e->modifiers().testFlag(Qt::ShiftModifier);
    if (ctrlOrShift && e->text().isEmpty())
    {
        return;
    }

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

void CommandLine::addToHistory(const QString &cmd)
{
    if (!cmd.isEmpty())
    {
        m_historyPosition = 0;

        m_history.removeAll(cmd);
        m_history.prepend(cmd);
    }
    else
    {
        m_historyPosition = -1;
    }
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
        m_historyPosition--;
        m_historyPosition = qBound(0, m_historyPosition, m_history.length() - 1);

        setPlainText(m_history.at(m_historyPosition));
        selectAll();
    }
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

    m_completionModel->setStringList(QStringList() << "sample one" << "sample two" << "samwise gamgee");
}

void CommandLine::processCommand(const QString &text)
{
    if (text.isEmpty())
    {
        emit command("");
        return;
    }

    if (!m_scriptPrefix.isEmpty() && text.startsWith(m_scriptPrefix))
    {
        emit script(text.mid(m_scriptPrefix.length()));
    }
    else
    {
        QRegularExpression regex("(?:" + QRegularExpression::escape(m_commandSeparator) + "|\\n)");
        QStringList cmds(text.split(regex));
        foreach (QString cmd, cmds)
        {
            emit command(cmd);
        }
    }

    if (m_clearCommandLine || !m_echoOn)
    {
        clear();
    }
    else
    {
        selectAll();
    }
}
