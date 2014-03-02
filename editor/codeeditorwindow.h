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


#ifndef CODEEDITORWINDOW_H
#define CODEEDITORWINDOW_H

#include "editor_global.h"
#include <QMainWindow>

class CodeEditorWidget;
class SearchWidget;
class QAction;
class QMdiArea;
class QMdiSubWindow;
class QPrinter;
class QToolBar;

class EDITORSHARED_EXPORT CodeEditorWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CodeEditorWindow(QWidget *parent = 0);

    void newEditor();
    void openEditor(const QString &fileName = QString());
    void saveEditor();
    void addEditor(CodeEditorWidget *editor);
    CodeEditorWidget * activeEditor();

public slots:
    void actionNew();
    void actionOpen();
    void actionSave();
    void actionSaveAs();
    void actionClose();
    void actionPrint();
    void actionPrintPreview();
    void printPreview(QPrinter *printer);
    void showSearchBox();

private slots:
    void updateActions();
    void editorModified();

private:
    QMdiArea *m_mdi;
    QToolBar *m_toolBar;

    QAction *m_actionNew;
    QAction *m_actionOpen;
    QAction *m_actionSave;
    QAction *m_actionSaveAs;
    QAction *m_actionClose;
    QAction *m_actionPrint;
    QAction *m_actionPrintPreview;

    SearchWidget *m_searchWidget;
};

#endif // CODEEDITORWINDOW_H
