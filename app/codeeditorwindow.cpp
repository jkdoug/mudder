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


#include "codeeditorwindow.h"
#include "luahighlighter.h"
#include "options.h"
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiArea>
#include <QMessageBox>
#include <QTabBar>
#include <QToolBar>

struct CodeEditorWindowData
{
    CodeEditorWindowData() :
        mdi(0),
        subWindow(0),
        toolBar(0)
    {}

    QMdiArea * mdi;
    QMdiSubWindow * subWindow;
    QToolBar * toolBar;

    QString defaultPath;
};

CodeEditorWindow::CodeEditorWindow(QWidget *parent) :
    QMainWindow(parent)
{
    d = new CodeEditorWindowData;

    d->defaultPath = Options::homePath();

    d->mdi = new QMdiArea(this);
    d->mdi->setDocumentMode(true);
    d->mdi->setViewMode(QMdiArea::TabbedView);
    d->mdi->setTabsClosable(true);
    d->mdi->setTabsMovable(true);
    d->mdi->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, false);

    setCentralWidget(d->mdi);

    d->toolBar = new QToolBar(this);
    d->toolBar->setIconSize(QSize(16, 16));

    QAction * actionNew = new QAction(tr("&New"), this);
    actionNew->setIcon(QIcon(":/icons/small_new"));
    actionNew->setToolTip(tr("Create a new file"));
    actionNew->setShortcut(QKeySequence::New);
    actionNew->setShortcutContext(Qt::WindowShortcut);
    d->toolBar->addAction(actionNew);

    QAction * actionOpen = new QAction(tr("&Open"), this);
    actionOpen->setIcon(QIcon(":/icons/small_open"));
    actionOpen->setToolTip(tr("Load file from disk"));
    actionOpen->setShortcut(QKeySequence::Open);
    actionOpen->setShortcutContext(Qt::WindowShortcut);
    d->toolBar->addAction(actionOpen);

    addToolBar(d->toolBar);

    QTabBar * mdiTabBar = d->mdi->findChild<QTabBar *>();
    Q_ASSERT(mdiTabBar != 0);
    mdiTabBar->setExpanding(false);

    connect(d->mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowActivated(QMdiSubWindow*)));
    connect(actionNew, SIGNAL(triggered()), this, SLOT(actionNew()));
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));
}

QString CodeEditorWindow::defaultPath() const
{
    return d->defaultPath;
}

void CodeEditorWindow::setDefaultPath(const QString &path)
{
    d->defaultPath = path;
}

void CodeEditorWindow::actionNew()
{
    static int num = 0;

    CodeEditorWidget * widget = new CodeEditorWidget(this);

    QMdiSubWindow * win = new QMdiSubWindow(this);
    win->setWidget(widget);
    win->setWindowTitle(tr("(untitled-%1)").arg(++num));
    win->setAttribute(Qt::WA_DeleteOnClose);
    widget->setSyntaxHighlighter(new LuaHighlighter());

    d->mdi->addSubWindow(win);

    win->show();

//    connect(script->document(), SIGNAL(contentsChanged()), this, SLOT(onScriptChanged()));

//    updateActions();
}

void CodeEditorWindow::actionOpen()
{
    QString path(d->defaultPath);

    QString fileName(QFileDialog::getOpenFileName(this, tr("Open File"), path, tr("Lua script files (*.lua);;All files (*)")));
    if (!fileName.isEmpty())
    {
        CodeEditorWidget * widget = new CodeEditorWidget(this);
        if (!widget->loadFile(fileName))
        {
            Options::removeRecentFile(fileName);
//            updateRecentFiles();

            QMessageBox::warning(this, tr("File Error"), tr("Failed to load file."));

            delete widget;
            return;
        }

//        if (!ui->scriptDock->isVisible())
//        {
//            ui->scriptDock->show();
//        }

//        setCurrentFile(filename);
//        statusBar()->showMessage(tr("Script loaded"), 2000);

//        connect(script->document(), SIGNAL(contentsChanged()), this, SLOT(onScriptChanged()));

        QMdiSubWindow *win = new QMdiSubWindow(this);
        win->setWidget(widget);
        win->setWindowTitle(QFileInfo(fileName).fileName());
        win->setAttribute(Qt::WA_DeleteOnClose);
        widget->setSyntaxHighlighter(new LuaHighlighter());

        d->mdi->addSubWindow(win);

        win->show();
    }
}

bool CodeEditorWindow::actionSave()
{
    return true;

//    if (!d->fileName.isEmpty())
//    {
//        return saveFile();
//    }

//    return actionSaveAs();
}

bool CodeEditorWindow::actionSaveAs()
{
    return true;

//    QString path(d->defaultPath);

//    if (!d->fileName.isEmpty())
//    {
//        QFileInfo fi(d->current_file);
//        QDir dir(fi.path());
//        if (dir.exists())
//            start_path = fi.filePath();
//    }

//    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As..."), start_path, tr("All Files (*)"));
//    if (fileName.isEmpty())
//        return false;

//    return saveFile(fileName);
}

void CodeEditorWindow::actionPrint()
{

}

void CodeEditorWindow::actionPrintPreview()
{

}

void CodeEditorWindow::actionPrintPdf()
{

}

void CodeEditorWindow::showEditorSettings()
{

}

void CodeEditorWindow::showSearchBox()
{

}

void CodeEditorWindow::printPreview(QPrinter *printer)
{

}

void CodeEditorWindow::updateSaveAction()
{

}

void CodeEditorWindow::subWindowActivated(QMdiSubWindow *win)
{
    d->subWindow = win;
}
