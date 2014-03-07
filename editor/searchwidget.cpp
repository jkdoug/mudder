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


#include "searchwidget.h"
#include "ui_searchwidget.h"

SearchWidget::SearchWidget(SearchOptions options, SearchMode mode,
                           SearchButtons buttons, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchWidget)
{
    ui->setupUi(this);

    setWindowTitle(tr("Search Widget"));

    int labelWidth = fontMetrics().width(ui->labelReplace->text());
    ui->labelFind->setMinimumWidth(labelWidth);
    ui->labelReplace->setMinimumWidth(labelWidth);

    connect(ui->findPrevious, SIGNAL(clicked()), SLOT(handleFindPrevious()));
    connect(ui->findNext, SIGNAL(clicked()), SLOT(handleFindNext()));
    connect(ui->replacePrevious, SIGNAL(clicked()), SLOT(handleReplacePrevious()));
    connect(ui->replaceNext, SIGNAL(clicked()), SLOT(handleReplaceNext()));
    connect(ui->replaceAll, SIGNAL(clicked()), SLOT(handleReplaceAll()));
    connect(ui->close, SIGNAL(clicked()), SLOT(handleClose()));
    connect(ui->searchString, SIGNAL(textChanged(QString)), SLOT(handleSearchStringChanged(QString)));
    connect(ui->replaceString, SIGNAL(textChanged(QString)), SLOT(handleReplaceStringChanged(QString)));

    m_searchOptionsMenu = new QMenu(tr("Search Options"), this);
    m_searchOptionsMenu->setObjectName(objectName());

    m_searchCaseSensitive = new QAction(tr("Case Sensitive"), this);
    m_searchCaseSensitive->setCheckable(true);
    m_searchCaseSensitive->setObjectName(objectName());
    m_searchOptionsMenu->addAction(m_searchCaseSensitive);
    connect(m_searchCaseSensitive, SIGNAL(triggered()), SLOT(handleOptionsChanged()));

    m_searchWholeWordsOnly = new QAction(tr("Whole Words Only"), this);
    m_searchWholeWordsOnly->setCheckable(true);
    m_searchWholeWordsOnly->setObjectName(objectName());
    m_searchOptionsMenu->addAction(m_searchWholeWordsOnly);
    connect(m_searchWholeWordsOnly, SIGNAL(triggered()), SLOT(handleOptionsChanged()));

    m_searchOptionsMenu->addSeparator();

    m_searchRegEx = new QAction(tr("Regular Expression"), this);
    m_searchRegEx->setObjectName(objectName());
    m_searchRegEx->setCheckable(true);
    m_searchOptionsMenu->addAction(m_searchRegEx);

    m_searchWildcard = new QAction(tr("Wildcard Expression"), this);
    m_searchWildcard->setCheckable(true);
    m_searchWildcard->setObjectName(objectName());
    m_searchOptionsMenu->addAction(m_searchWildcard);

    m_searchFixedString = new QAction(tr("Fixed String"), this);
    m_searchFixedString->setCheckable(true);
    m_searchFixedString->setObjectName(objectName());
    m_searchOptionsMenu->addAction(m_searchFixedString);

    m_searchPatternGroup = new QActionGroup(this);
    m_searchPatternGroup->addAction(m_searchRegEx);
    m_searchPatternGroup->addAction(m_searchWildcard);
    m_searchPatternGroup->addAction(m_searchFixedString);
    m_searchWildcard->setChecked(true);

    connect(m_searchPatternGroup, SIGNAL(triggered(QAction*)), SLOT(handleOptionsChanged()));

    ui->searchOptions->setPopupMode(QToolButton::InstantPopup);
    ui->searchOptions->setMenu(m_searchOptionsMenu);

    setSearchButtons(buttons);
    setSearchOptions(options);
    setSearchMode(mode);
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

QString SearchWidget::currentSearchString() const
{
    return ui->searchString->text();
}

void SearchWidget::setCurrentSearchString(const QString &search)
{
    ui->searchString->setText(search);
}

QString SearchWidget::currentReplaceString() const
{
    return ui->replaceString->text();
}

void SearchWidget::setCurrentReplaceString(const QString &replace)
{
    ui->replaceString->setText(replace);
}

void SearchWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);

    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        break;
    }
}

bool SearchWidget::caseSensitive() const
{
    if (m_searchCaseSensitive)
    {
        return m_searchCaseSensitive->isChecked();
    }

    return false;
}

bool SearchWidget::wholeWordsOnly() const
{
    if (m_searchWholeWordsOnly)
    {
        return m_searchWholeWordsOnly->isChecked();
    }

    return false;
}

void SearchWidget::setCaseSensitive(bool toggle)
{
    if (m_searchCaseSensitive)
    {
        m_searchCaseSensitive->setChecked(toggle);
    }
}

void SearchWidget::setWholeWordsOnly(bool toggle)
{
    if (m_searchWholeWordsOnly)
    {
        m_searchWholeWordsOnly->setChecked(toggle);
    }
}

void SearchWidget::setEditorFocus(bool selectText)
{
    ui->searchString->setFocus();
    if (selectText)
    {
        ui->searchString->selectAll();
    }
}

void SearchWidget::setSearchButtons(SearchButtons buttons)
{
    m_buttons = buttons;

    if (m_buttons.testFlag(NextButtons))
    {
        ui->findNext->show();
        ui->replaceNext->show();
    }
    else
    {
        ui->findNext->hide();
        ui->replaceNext->hide();
    }

    if (m_buttons & PreviousButtons)
    {
        ui->findPrevious->show();
        ui->replacePrevious->show();
    }
    else
    {
        ui->findPrevious->hide();
        ui->replacePrevious->hide();
    }

    if (m_buttons.testFlag(HideButtonDown) || m_buttons.testFlag(HideButtonUp))
    {
        if (m_buttons.testFlag(HideButtonDown))
        {
            ui->close->setArrowType(Qt::DownArrow);
        }
        else
        {
            ui->close->setArrowType(Qt::UpArrow);
        }
        ui->close->show();
    }
    else
    {
        ui->close->hide();
    }
}

void SearchWidget::setSearchOptions(SearchOptions options)
{
    m_options = options;
    if (m_options.testFlag(CaseSensitive))
    {
        m_searchCaseSensitive->setVisible(true);
        m_searchCaseSensitive->setChecked(true);
    }
    else
    {
        m_searchCaseSensitive->setVisible(false);
    }

    if (m_options.testFlag(WholeWordsOnly))
    {
        m_searchWholeWordsOnly->setVisible(true);
        m_searchWholeWordsOnly->setChecked(true);
    }
    else
    {
        m_searchWholeWordsOnly->setVisible(false);
    }

    if (m_options.testFlag(RegEx))
    {
        m_searchRegEx->setVisible(true);
    }
    if (m_options.testFlag(RegWildcard))
    {
        m_searchWildcard->setVisible(true);
    }
    if (m_options.testFlag(RegFixedString))
    {
        m_searchFixedString->setVisible(true);
    }
}

void SearchWidget::setSearchMode(SearchMode mode)
{
    m_mode = mode;
    if (m_mode == SearchOnly)
    {
        ui->widgetReplace->hide();
        setFixedHeight(26);
    }
    else if (m_mode == SearchAndReplace)
    {
        ui->widgetReplace->show();
        setFixedHeight(52);
    }
}

QMenu * SearchWidget::searchOptionsMenu()
{
    return m_searchOptionsMenu;
}

void SearchWidget::setTextEditor(QTextEdit * textEdit)
{
    m_textEdit = textEdit;

    if (m_textEdit)
    {
        m_target = SearchWidget::TextEdit;
    }
}

QTextEdit * SearchWidget::textEditor() const
{
    return m_textEdit;
}

void SearchWidget::setPlainTextEditor(QPlainTextEdit * plainTextEdit)
{
    m_plainTextEdit = plainTextEdit;

    if (m_plainTextEdit)
    {
        m_target = SearchWidget::PlainTextEdit;
    }
}

QPlainTextEdit * SearchWidget::plainTextEditor() const
{
    return m_plainTextEdit;
}

void SearchWidget::handleOptionsChanged()
{
    handleFindPrevious();
    handleFindNext();
    emit searchOptionsChanged();
}

void SearchWidget::handleReplaceStringChanged(const QString &string)
{
    emit replaceStringChanged(string);
}

void SearchWidget::handleClose()
{
    if (m_target == ExternalTarget)
    {
        emit close_clicked();
    }
    else if (m_target == TextEdit || m_target == PlainTextEdit)
    {
        emit close_clicked();
        hide();
    }
}

void SearchWidget::handleFindNext()
{
    if (m_target == ExternalTarget)
    {
        emit findNext_clicked();
    }
    else if (m_target == TextEdit && m_textEdit)
    {
        QTextDocument::FindFlags flags = findFlags();

        if (!m_textEdit->find(currentSearchString(), flags))
        {
            QTextCursor cursor(m_textEdit->textCursor());
            cursor.movePosition(QTextCursor::Start);
            m_textEdit->setTextCursor(cursor);
            m_textEdit->find(currentSearchString(), flags);
        }
    }
    else if (m_target == PlainTextEdit && m_plainTextEdit)
    {
        QTextDocument::FindFlags flags = findFlags();

        if (m_plainTextEdit->find(currentSearchString(), flags))
        {
            m_plainTextEdit->centerCursor();
        }
        else
        {
            QTextCursor cursor(m_plainTextEdit->textCursor());
            cursor.movePosition(QTextCursor::Start);
            m_plainTextEdit->setTextCursor(cursor);
            m_plainTextEdit->find(currentSearchString(), flags);
        }
    }
}

void SearchWidget::handleFindPrevious()
{
    if (m_target == ExternalTarget)
    {
        emit findPrevious_clicked();
    }
    else if (m_target == TextEdit && m_textEdit)
    {
        if (!m_textEdit->find(currentSearchString(), findFlags() | QTextDocument::FindBackward))
        {
            QTextCursor cursor(m_textEdit->textCursor());
            cursor.movePosition(QTextCursor::End);
            m_textEdit->setTextCursor(cursor);
            m_textEdit->find(currentSearchString(), findFlags() | QTextDocument::FindBackward);
        }
    }
    else if (m_target == PlainTextEdit && m_plainTextEdit)
    {
        if (m_plainTextEdit->find(currentSearchString(), findFlags() | QTextDocument::FindBackward))
        {
            m_plainTextEdit->centerCursor();
        }
        else
        {
            QTextCursor cursor = m_plainTextEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            m_plainTextEdit->setTextCursor(cursor);
            m_plainTextEdit->find(currentSearchString(), findFlags() | QTextDocument::FindBackward);
        }
    }
}

void SearchWidget::handleReplaceNext()
{
    if (m_target == ExternalTarget)
    {
        emit replaceNext_clicked();
    }
    else if (m_target == TextEdit && m_textEdit)
    {
        if (!m_textEdit->textCursor().hasSelection())
        {
            handleFindNext();
        }
        else
        {
            m_textEdit->textCursor().insertText(ui->replaceString->text());
            handleFindNext();
        }
    }
    else if (m_target == PlainTextEdit && m_plainTextEdit)
    {
        if (!m_plainTextEdit->textCursor().hasSelection())
        {
            handleFindNext();
        }
        else
        {
            m_plainTextEdit->textCursor().insertText(ui->replaceString->text());
            handleFindNext();
        }
    }
}

void SearchWidget::handleReplacePrevious()
{
    if (m_target == ExternalTarget)
    {
        emit replacePrevious_clicked();
    }
    else if (m_target == TextEdit && m_textEdit)
    {
        if (!m_textEdit->textCursor().hasSelection())
        {
            handleFindPrevious();
        }
        else
        {
            m_textEdit->textCursor().insertText(ui->replaceString->text());
            handleFindPrevious();
        }
    }
    else if (m_target == PlainTextEdit && m_plainTextEdit)
    {
        if (!m_plainTextEdit->textCursor().hasSelection())
        {
            handleFindPrevious();
        }
        else
        {
            m_plainTextEdit->textCursor().insertText(ui->replaceString->text());
            handleFindPrevious();
        }
    }
}

void SearchWidget::handleReplaceAll()
{
    if (m_target == ExternalTarget)
    {
        emit replaceAll_clicked();
        return;
    }

    int count = 0;
    if (m_target == TextEdit && m_textEdit)
    {
        QTextCursor cursor(m_textEdit->textCursor());
        int position = cursor.position();
        cursor.setPosition(0);
        cursor.beginEditBlock();

        m_textEdit->setTextCursor(cursor);
        m_textEdit->find(currentSearchString(), findFlags());
        while (m_textEdit->textCursor().hasSelection())
        {
            m_textEdit->textCursor().insertText(ui->replaceString->text());
            m_textEdit->find(currentSearchString(), findFlags());
            count++;
        }

        cursor.endEditBlock();
        cursor.setPosition(position);
        m_textEdit->setTextCursor(cursor);
    }
    else if (m_target == PlainTextEdit && m_plainTextEdit)
    {
        QTextCursor cursor(m_plainTextEdit->textCursor());
        int position = cursor.position();
        cursor.setPosition(0);
        cursor.beginEditBlock();

        m_plainTextEdit->setTextCursor(cursor);
        m_plainTextEdit->find(currentSearchString(), findFlags());
        while (m_plainTextEdit->textCursor().hasSelection())
        {
            m_plainTextEdit->textCursor().insertText(ui->replaceString->text());
            m_plainTextEdit->find(currentSearchString(), findFlags());
            count++;
        }

        cursor.endEditBlock();
        cursor.setPosition(position);
        m_plainTextEdit->setTextCursor(cursor);
    }
    else
    {
        return;
    }

    if (count == 1)
    {
        setMessage("<font color='green'>Replaced 1 occurence.</font>");
    }
    else if (count > 1)
    {
        setMessage(QString("<font color='green'>Replaced %1 occurences.</font>").arg(count));
    }
    else
    {
        setMessage("<font color='orange'>No matches found.</font>");
    }
}

QTextDocument::FindFlags SearchWidget::findFlags() const
{
    QTextDocument::FindFlags flags = 0;
    if (wholeWordsOnly())
    {
        flags |= QTextDocument::FindWholeWords;
    }
    if (caseSensitive())
    {
        flags |= QTextDocument::FindCaseSensitively;
    }
    return flags;
}

void SearchWidget::setInfoText(const QString &text)
{
    ui->infoMessage->setText(text);
}

QString SearchWidget::infoText() const
{
    return ui->infoMessage->text();
}

void SearchWidget::clearInfoText()
{
    ui->infoMessage->clear();
}

void SearchWidget::setNotificationMode(SearchWidget::NotificationMode mode)
{
    m_notificationMode = mode;
}

SearchWidget::NotificationMode SearchWidget::notificationMode() const
{
    return m_notificationMode;
}

void SearchWidget::setMessage(const QString &message)
{
    ui->labelMessage->setText(message);
}

void SearchWidget::on_searchString_returnPressed()
{
    if (m_notificationMode == NotifyOnReturn)
    {
        handleFindPrevious();
        handleFindNext();
        emit searchStringChanged(ui->searchString->text());
    }
}

void SearchWidget::handleSearchStringChanged(const QString &string)
{
    if (m_notificationMode == NotifyOnChange)
    {
        handleFindPrevious();
        handleFindNext();
        emit searchStringChanged(string);
    }
}
