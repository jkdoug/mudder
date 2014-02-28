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


#include "codeeditorwidget.h"
#include "coreapplication.h"
#include "coresettings.h"
#include "logger.h"
#include "luahighlighter.h"
#include "xmlhighlighter.h"
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMessageBox>

QString CodeEditorWidget::m_fileFilter(tr("Lua script files (*.lua);;XML source files (*.xml *.mp);;All files (*)"));

CodeEditorWidget::CodeEditorWidget(QWidget *parent) :
    CodeEditor(parent)
{
    m_watcher = new QFileSystemWatcher(this);

    connect(m_watcher, SIGNAL(fileChanged(QString)), SLOT(handleFileChange(QString)));
    connect(document(), SIGNAL(modificationChanged(bool)), SLOT(contentsModified(bool)));
}

CodeEditorWidget::~CodeEditorWidget()
{
    delete m_watcher;
}

void CodeEditorWidget::newFile()
{
    static int fileNumber = 1;

    m_fileName = tr("document%1").arg(fileNumber++);
    setWindowTitle(m_fileName + "[*]");
    m_isUntitled = true;

    LOG_INFO("Created a new file:", m_fileName);
}

bool CodeEditorWidget::save()
{
    LOG_TRACE("CodeEditorWidget::save", QString("Untitled: %1").arg(m_isUntitled), m_fileName);

    if (m_isUntitled)
    {
        return saveAs();
    }

    return saveFile(m_fileName);
}

bool CodeEditorWidget::saveAs()
{
    QString fileName(QFileDialog::getSaveFileName(this, tr("Save File"), m_fileName, m_fileFilter));
    LOG_TRACE("CodeEditorWidget::saveAs", fileName);
    if (fileName.isEmpty())
    {
        return false;
    }

    return saveFile(fileName);
}

CodeEditorWidget * CodeEditorWidget::open(QWidget *parent)
{
    QString fileName(QFileDialog::getOpenFileName(parent, tr("Open File"), QString(), m_fileFilter));
    LOG_TRACE("CodeEditorWidget::open", fileName);
    if (fileName.isEmpty())
    {
        return 0;
    }

    return openFile(fileName, parent);
}

CodeEditorWidget * CodeEditorWidget::openFile(const QString &fileName, QWidget *parent)
{
    LOG_TRACE("CodeEditorWidget::openFile", fileName);

    CodeEditorWidget *editor = new CodeEditorWidget(parent);
    if (editor->readFile(fileName))
    {
        editor->setCurrentFile(fileName);
        return editor;
    }

    delete editor;
    return 0;
}

void CodeEditorWidget::handleFileChange(const QString &path)
{
    if (!m_watcherMutex.tryLock())
    {
        return;
    }

    QMessageBox msgBox(QMessageBox::Question, tr("File Changed"), QString(), QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    QFile file(path);
    if (file.exists())
    {
        msgBox.setText(tr("Your file has changed outside of the editor:<br><br>") + m_fileName);
        msgBox.setInformativeText(tr("Do you want to reload the file?"));

        int ret = msgBox.exec();
        switch (ret)
        {
        case QMessageBox::Yes:
            setWindowModified(false);
            readFile(path);
            break;

        case QMessageBox::No:
            setWindowModified(true);
            break;

        default:
            break;
        }
    }
    else
    {
        msgBox.setText(tr("Your file has been removed outside of the editor:<br><br>") + m_fileName);
        msgBox.setInformativeText(tr("Do you want to keep this file open in the editor?"));

        int ret = msgBox.exec();
        switch (ret)
        {
        case QMessageBox::Yes:
            setWindowModified(true);
            break;

        case QMessageBox::No:
            setWindowModified(false);
            close();
            break;

        default:
            break;
        }
    }

    m_watcherMutex.unlock();
}

void CodeEditorWidget::contentsModified(bool changed)
{
    setWindowModified(changed);
}

void CodeEditorWidget::closeEvent(QCloseEvent *e)
{
    if (!okToContinue())
    {
        e->ignore();
    }
    else
    {
        e->accept();
    }
}

bool CodeEditorWidget::okToContinue()
{
    if (isWindowModified())
    {
        QMessageBox::StandardButton answer = QMessageBox::warning(this, tr("Editor"),
            tr("The file has been modified.\nDo you want to save your changes?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Yes)
        {
            return save();
        }
        else if (answer == QMessageBox::Cancel)
        {
            return false;
        }
    }

    return true;
}

bool CodeEditorWidget::saveFile(const QString &fileName)
{
    LOG_TRACE("CodeEditorWidget::saveFile", fileName);

    if (writeFile(fileName))
    {
        setCurrentFile(fileName);
        return true;
    }

    return false;
}

void CodeEditorWidget::setCurrentFile(const QString &fileName)
{
    LOG_TRACE("CodeEditorWidget::setCurrentFile", fileName);

    m_fileName = fileName;
    m_isUntitled = false;

    QString suffix(QFileInfo(fileName).suffix());
    if (suffix.compare("lua", Qt::CaseInsensitive) == 0)
    {
        setSyntaxHighlighter(new LuaHighlighter);
    }
    else if (suffix.compare("xml", Qt::CaseInsensitive) == 0 ||
             suffix.compare("mp", Qt::CaseInsensitive) == 0)
    {
        setSyntaxHighlighter(new XmlHighlighter);
    }

    setWindowTitle(QFileInfo(fileName).fileName() + "[*]");
    setWindowModified(false);
}

bool CodeEditorWidget::readFile(const QString &fileName)
{
    LOG_TRACE("CodeEditorWidget::readFile", fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Editor"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));

        LOG_WARNING("Cannot read file:", fileName, file.errorString());
        return false;
    }

    CoreApplication::setApplicationBusy(true);

    setPlainText(file.readAll());
    setWindowModified(false);

    QString previous(m_fileName);
    m_fileName = fileName;

    if (m_watcher->files().contains(previous))
    {
        m_watcher->removePath(previous);
    }
    m_watcher->addPath(m_fileName);

    CoreApplication::setApplicationBusy(false);

    return true;
}

bool CodeEditorWidget::writeFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Editor"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));

        LOG_WARNING("Cannot write file:", fileName, file.errorString());
        return false;
    }

    CoreApplication::setApplicationBusy(true);

    LOG_INFO("Writing file:", fileName);

    if (m_watcher->files().contains(fileName))
    {
        m_watcher->removePath(fileName);
    }

    file.write(toPlainText().toUtf8());
    file.close();

    m_fileName = fileName;

    m_watcher->addPath(m_fileName);

    CoreApplication::setApplicationBusy(false);

    return true;
}
