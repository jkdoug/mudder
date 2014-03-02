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


#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "editor_global.h"
#include <QWidget>
#include <QMenu>
#include <QTextEdit>
#include <QPlainTextEdit>

namespace Ui {
class SearchWidget;
}

class EDITORSHARED_EXPORT SearchWidget : public QWidget
{
    Q_OBJECT

public:
    enum SearchMode {
        SearchOnly,
        SearchAndReplace
    };
    Q_ENUMS(SearchMode)

    enum NotificationMode {
        NotifyOnChange,
        NotifyOnReturn
    };
    Q_ENUMS(NotificationMode)

    enum SearchTarget {
        ExternalTarget,
        TextEdit,
        PlainTextEdit
    };
    Q_ENUMS(SearchTarget)

    enum SearchButton {
        NoButtons = 0,
        NextButtons = 1 << 0,
        PreviousButtons = 1 << 1,
        HideButtonUp = 1 << 2,
        HideButtonDown = 1 << 3,
        AllButtons = NextButtons | PreviousButtons | HideButtonDown
    };
    Q_ENUMS(SearchButton)
    Q_DECLARE_FLAGS(SearchButtons, SearchButton)
    Q_FLAGS(SearchButtons)

    enum SearchOption {
        NoOptions = 0,
        CaseSensitive = 1 << 0,
        WholeWordsOnly = 1 << 1,
        RegEx = 1 << 2,
        RegFixedString = 1 << 3,
        RegWildcard = 1 << 4,
        AllSearchOptions = CaseSensitive | WholeWordsOnly | RegEx
    };
    Q_ENUMS(SearchOption)
    Q_DECLARE_FLAGS(SearchOptions, SearchOption)
    Q_FLAGS(SearchOptions)

    SearchWidget(SearchOptions options = AllSearchOptions,
                 SearchMode mode = SearchOnly,
                 SearchButtons buttons = AllButtons,
                 QWidget *parent = 0);
    ~SearchWidget();

    QString currentSearchString() const;
    QString currentReplaceString() const;
    bool caseSensitive() const;
    bool wholeWordsOnly() const;
    void setCaseSensitive(bool toggle);
    void setWholeWordsOnly(bool toggle);
    void setEditorFocus(bool selectText = true);
    void setTextEditor(QTextEdit * textEdit);
    QTextEdit * textEditor() const;
    void setPlainTextEditor(QPlainTextEdit * plainTextEdit);
    QPlainTextEdit * plainTextEditor() const;
    SearchTarget searchTarget() const { return m_target; }

    void setSearchButtons(SearchButtons buttons);
    SearchButtons searchButtons() const { return m_buttons; }
    void setSearchOptions(SearchOptions options);
    SearchOptions searchOptions() const { return m_options; }
    void setSearchMode(SearchMode mode);
    SearchMode searchMode() const { return m_mode; }
    QMenu * searchOptionsMenu();
    void setMessage(const QString &message);
    QTextDocument::FindFlags findFlags() const;

    void setInfoText(const QString &text);
    QString infoText() const;
    void clearInfoText();

    void setNotificationMode(NotificationMode mode);
    NotificationMode notificationMode() const;

public slots:
    void setCurrentSearchString(const QString &search);
    void setCurrentReplaceString(const QString &replace);

protected:
    void changeEvent(QEvent *e);

private slots:
    void handleReplaceStringChanged(const QString &string);
    void handleSearchStringChanged(const QString &string);
    void handleOptionsChanged();
    void handleClose();

    void on_searchString_returnPressed();

public slots:
    void handleFindNext();
    void handleFindPrevious();
    void handleReplaceNext();
    void handleReplacePrevious();
    void handleReplaceAll();

signals:
    void searchStringChanged(const QString &string);
    void replaceStringChanged(const QString &string);
    void searchOptionsChanged();
    void close_clicked();
    void findPrevious_clicked();
    void findNext_clicked();
    void replacePrevious_clicked();
    void replaceNext_clicked();
    void replaceAll_clicked();

private:
    Ui::SearchWidget *ui;

    QMenu *m_searchOptionsMenu;
    QAction *m_searchCaseSensitive;
    QAction *m_searchWholeWordsOnly;

    QAction *m_searchRegEx;
    QAction *m_searchFixedString;
    QAction *m_searchWildcard;
    QActionGroup *m_searchPatternGroup;

    SearchTarget m_target;
    QTextEdit *m_textEdit;
    QPlainTextEdit *m_plainTextEdit;

    SearchButtons m_buttons;
    SearchOptions m_options;
    SearchMode m_mode;
    NotificationMode m_notificationMode;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SearchWidget::SearchButtons)
Q_DECLARE_OPERATORS_FOR_FLAGS(SearchWidget::SearchOptions)

#endif // SEARCHWIDGET_H
