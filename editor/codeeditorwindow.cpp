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


#include "codeeditorwindow.h"
#include "codeeditorwidget.h"
#include "luahighlighter.h"
#include "coresettings.h"
#include "coreapplication.h"
#include "contextmanager.h"
#include "logger.h"
#include "xmlhighlighter.h"
#include "searchwidget.h"
#include "proxyaction.h"
#include <QAction>
#include <QBoxLayout>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QTabBar>
#include <QToolBar>
#include <QtDebug>

CodeEditorWindow::CodeEditorWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QString contextName("lib.core.CodeEditorWindow");
    int count = 1;
    while (CONTEXT_MANAGER->hasContext(contextName + QString::number(count)))
    {
        count++;
    }
    setObjectName(contextName + QString::number(count));
    CONTEXT_MANAGER->registerContext(objectName());

    m_mdi = new QMdiArea(this);
    m_mdi->setDocumentMode(true);
    m_mdi->setViewMode(QMdiArea::TabbedView);
    m_mdi->setTabsClosable(true);
    m_mdi->setTabsMovable(true);
    m_mdi->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, false);
    connect(m_mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), SLOT(updateActions()));

    m_mdi->installEventFilter(this);

    QWidget *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    if (centralWidget->layout())
    {
        delete centralWidget->layout();
    }

    QLayout *centralLayout = new QBoxLayout(QBoxLayout::TopToBottom, centralWidget);
    centralLayout->addWidget(m_mdi);
    centralLayout->setMargin(0);
    centralLayout->setSpacing(0);

    m_searchWidget = 0;

    m_toolBar = new QToolBar(this);
    m_toolBar->setIconSize(QSize(16, 16));

    m_actionNew = new QAction(tr("&New"), this);
    m_actionNew->setIcon(QIcon(":/icons/small_new"));
    m_actionNew->setToolTip(tr("Create new file"));
    m_actionNew->setShortcut(QKeySequence::New);
    m_actionNew->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionNew);
    connect(m_actionNew, SIGNAL(triggered()), SLOT(actionNew()));

    m_actionOpen = new QAction(tr("&Open"), this);
    m_actionOpen->setIcon(QIcon(":/icons/small_open"));
    m_actionOpen->setToolTip(tr("Load file from disk"));
    m_actionOpen->setShortcut(QKeySequence::Open);
    m_actionOpen->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionOpen);
    connect(m_actionOpen, SIGNAL(triggered()), SLOT(actionOpen()));

    m_actionSave = new QAction(tr("&Save"), this);
    m_actionSave->setIcon(QIcon(":/icons/small_save"));
    m_actionSave->setToolTip(tr("Save file to disk"));
    m_actionSave->setShortcut(QKeySequence::Save);
    m_actionSave->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionSave);
    connect(m_actionSave, SIGNAL(triggered()), SLOT(actionSave()));

    m_actionSaveAs = new QAction(tr("Save &As"), this);
    m_actionSaveAs->setIcon(QIcon(":/icons/small_save_as"));
    m_actionSaveAs->setToolTip(tr("Save file to disk with a new name"));
    m_actionSaveAs->setShortcut(QKeySequence::SaveAs);
    m_actionSaveAs->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionSaveAs);
    connect(m_actionSaveAs, SIGNAL(triggered()), SLOT(actionSaveAs()));

    m_actionClose = new QAction(tr("Close"), this);
    m_actionClose->setIcon(QIcon(":/icons/small_close"));
    m_actionClose->setToolTip(tr("Close the current file"));
    m_actionClose->setShortcut(QKeySequence::Close);
    m_actionClose->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionClose);
    connect(m_actionClose, SIGNAL(triggered()), SLOT(actionClose()));

    m_toolBar->addSeparator();

    m_actionCopy = new QAction(tr("&Copy"), this);
    m_actionCopy->setIcon(QIcon(":/icons/small_copy"));
    m_actionCopy->setToolTip(tr("Copy selection to the clipboard"));
    m_actionCopy->setShortcut(QKeySequence::Copy);
    m_actionCopy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionCopy);
    connect(m_actionCopy, SIGNAL(triggered()), SLOT(actionCopy()));

    m_actionCut = new QAction(tr("Cu&t"), this);
    m_actionCut->setIcon(QIcon(":/icons/small_cut"));
    m_actionCut->setToolTip(tr("Cut selection to the clipboard"));
    m_actionCut->setShortcut(QKeySequence::Cut);
    m_actionCut->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionCut);
    connect(m_actionCut, SIGNAL(triggered()), SLOT(actionCut()));

    m_actionPaste = new QAction(tr("&Paste"), this);
    m_actionPaste->setIcon(QIcon(":/icons/small_paste"));
    m_actionPaste->setToolTip(tr("Paste text from the clipboard"));
    m_actionPaste->setShortcut(QKeySequence::Paste);
    m_actionPaste->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionPaste);
    connect(m_actionPaste, SIGNAL(triggered()), SLOT(actionPaste()));

    m_toolBar->addSeparator();

    m_actionPrint = new QAction(tr("&Print"), this);
    m_actionPrint->setIcon(QIcon(":/icons/small_print"));
    m_actionPrint->setToolTip(tr("Print the file"));
    m_actionPrint->setShortcut(QKeySequence::Print);
    m_actionPrint->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_toolBar->addAction(m_actionPrint);
    connect(m_actionPrint, SIGNAL(triggered()), SLOT(actionPrint()));

    m_actionPrintPreview = new QAction(tr("Print Preview"), this);
    m_actionPrintPreview->setIcon(QIcon(":/icons/small_print_preview"));
    m_actionPrintPreview->setToolTip(tr("Preview the file before printing"));
    m_toolBar->addAction(m_actionPrintPreview);
    connect(m_actionPrintPreview, SIGNAL(triggered()), SLOT(actionPrintPreview()));

    m_toolBar->addSeparator();

    addToolBar(m_toolBar);

    QTabBar * mdiTabBar = m_mdi->findChild<QTabBar *>();
    mdiTabBar->setExpanding(false);
    mdiTabBar->setIconSize(QSize(0, 0));

    updateActions();

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(updateActions()));
}

void CodeEditorWindow::actionNew()
{
    newEditor();
}

void CodeEditorWindow::actionOpen()
{
    openEditor();
}

void CodeEditorWindow::actionSave()
{
    saveEditor();
}

void CodeEditorWindow::actionSaveAs()
{
    CodeEditorWidget *editor = activeEditor();
    if (editor)
    {
        if (editor->saveAs())
        {
            SETTINGS->addRecentFile(editor->fileName());
        }
    }
}

void CodeEditorWindow::actionClose()
{
    m_mdi->closeActiveSubWindow();
}

void CodeEditorWindow::actionCopy()
{
    CodeEditorWidget *editor = activeEditor();
    if (editor)
    {
        editor->copy();
    }
}

void CodeEditorWindow::actionCut()
{
    CodeEditorWidget *editor = activeEditor();
    if (editor)
    {
        editor->cut();
    }
}

void CodeEditorWindow::actionPaste()
{
    CodeEditorWidget *editor = activeEditor();
    if (editor)
    {
        editor->paste();
    }
}

void CodeEditorWindow::actionPrint()
{
#ifndef QT_NO_PRINTER
    CodeEditorWidget *editor = activeEditor();
    if (!editor)
    {
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(editor->fileName());

    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (editor->textCursor().hasSelection())
    {
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    }
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
    {
        editor->print(&printer);
    }
    delete dlg;
#endif
}

void CodeEditorWindow::actionPrintPreview()
{
#ifndef QT_NO_PRINTER
    CodeEditorWidget *editor = activeEditor();
    if (!editor)
    {
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(editor->fileName());

    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter *)), SLOT(printPreview(QPrinter *)));
    preview.exec();
#endif
}

void CodeEditorWindow::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    CodeEditorWidget *editor = activeEditor();
    if (editor)
    {
        editor->print(printer);
    }
#endif
}

void CodeEditorWindow::showSearchBox()
{
//    if (!m_searchWidget)
//    {
//        SearchWidget::SearchOptions options = SETTINGS->value("Editor/SearchOptions",
//                                                              SearchWidget::NoOptions).toInt();
//        SearchWidget::SearchButtons buttons = SETTINGS->value("Editor/SearchButtons",
//                                                              SearchWidget::AllButtons).toInt();
//        m_searchWidget = new SearchWidget(options, SearchWidget::SearchAndReplace, buttons);
//        m_searchWidget->setPlainTextEditor(activeEditor());

//        centralWidget()->layout()->addWidget(m_searchWidget);
//    }

//    m_searchWidget->setEditorFocus();

//    if (activeEditor())
//    {
//        QTextCursor cursor(activeEditor()->textCursor());
//        if (cursor.hasSelection())
//        {
//            m_searchWidget->setCurrentSearchString(cursor.selectedText());
//        }
//    }

//    if (!m_searchWidget->isVisible())
//    {
//        m_searchWidget->show();
//    }
}

void CodeEditorWindow::closeEvent(QCloseEvent *e)
{
    m_mdi->closeAllSubWindows();
    if (activeEditor())
    {
        e->ignore();
    }
    else
    {
        e->accept();
    }
}

void CodeEditorWindow::focusInEvent(QFocusEvent *e)
{
    LOG_TRACE("CodeEditorWindow::focusInEvent");

    CONTEXT_MANAGER->appendContext(objectName());

    QMainWindow::focusInEvent(e);
}

void CodeEditorWindow::focusOutEvent(QFocusEvent *e)
{
    LOG_TRACE("CodeEditorWindow::focusOutEvent");

    CONTEXT_MANAGER->removeContext(objectName());

    QMainWindow::focusOutEvent(e);
}

bool CodeEditorWindow::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)

    if (e->type() == QEvent::FocusIn)
    {
//        LOG_TRACE("CodeEditorWindow::eventFilter - FocusIn", obj->metaObject()->className());
        focusInEvent(static_cast<QFocusEvent *>(e));
    }
    else if (e->type() == QEvent::FocusOut)
    {
//        LOG_TRACE("CodeEditorWindow::eventFilter - FocusOut", obj->metaObject()->className());
        focusOutEvent(static_cast<QFocusEvent *>(e));
    }

    return false;
}

void CodeEditorWindow::updateActions()
{
    bool hasEditor = activeEditor() != 0;
    bool modified = hasEditor && activeEditor()->isWindowModified();
    bool selected = hasEditor && activeEditor()->textCursor().hasSelection();

    m_actionSave->setEnabled(hasEditor && modified);
    m_actionSaveAs->setEnabled(hasEditor);
    m_actionClose->setEnabled(hasEditor);
    m_actionCopy->setEnabled(selected);
    m_actionCut->setEnabled(selected);
    m_actionPaste->setEnabled(hasEditor && !QApplication::clipboard()->text().isEmpty());
    m_actionPrint->setEnabled(hasEditor);
    m_actionPrintPreview->setEnabled(hasEditor);

    if (m_searchWidget)
    {
        m_searchWidget->setPlainTextEditor(activeEditor());
    }
}

void CodeEditorWindow::newEditor()
{
    CodeEditorWidget *editor = new CodeEditorWidget;
    editor->newFile();
    addEditor(editor);
}

void CodeEditorWindow::openEditor(const QString &fileName)
{
    CodeEditorWidget *editor = 0;
    if (fileName.isEmpty())
    {
        editor = CodeEditorWidget::open(this);
    }
    else
    {
        editor = CodeEditorWidget::openFile(fileName, this);
    }

    if (editor)
    {
        SETTINGS->addRecentFile(editor->fileName());

        addEditor(editor);
    }
}

void CodeEditorWindow::saveEditor()
{
    CodeEditorWidget *editor = activeEditor();
    if (editor)
    {
        editor->save();
    }
}

void CodeEditorWindow::addEditor(CodeEditorWidget *editor)
{
    editor->installEventFilter(this);

    QMdiSubWindow *subWindow = m_mdi->addSubWindow(editor);
    subWindow->showMaximized();
    connect(editor, SIGNAL(modificationChanged(bool)), SLOT(updateActions()));
    connect(editor, SIGNAL(copyAvailable(bool)), SLOT(updateActions()));
}

CodeEditorWidget * CodeEditorWindow::activeEditor()
{
    QMdiSubWindow *subWindow = m_mdi->activeSubWindow();
    if (subWindow)
    {
        return qobject_cast<CodeEditorWidget *>(subWindow->widget());
    }

    return 0;
}
