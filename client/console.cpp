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


#include "console.h"
#include "ui_console.h"
#include "coreapplication.h"
#include "logger.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

Console::Console(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Console)
{
    ui->setupUi(this);

    m_action = new QAction(this);
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(triggered()), SLOT(show()));
    connect(m_action, SIGNAL(triggered()), SLOT(setFocus()));

    m_isUntitled = true;

    setWindowTitle("[*]");
    setAttribute(Qt::WA_DeleteOnClose);
}

Console::~Console()
{
    delete ui;
}

void Console::newFile()
{
    static int fileNumber = 1;

    m_fileName = tr("profile%1").arg(fileNumber++);
    setWindowTitle(m_fileName + "[*]");
    m_action->setText(m_fileName);
    m_isUntitled = true;

    LOG_INFO("Created a new profile:", m_fileName);
}

bool Console::save()
{
    if (m_isUntitled)
    {
        return saveAs();
    }

    return saveFile(m_fileName);
}

bool Console::saveAs()
{
    QString fileName(QFileDialog::getSaveFileName(this, tr("Save Profile"),
                                                  m_fileName, tr("Mudder Profiles (*.mp);;All files (*)")));
    if (fileName.isEmpty())
    {
        return false;
    }

    return saveFile(fileName);
}

Console * Console::open(QWidget *parent)
{
    QString fileName(QFileDialog::getOpenFileName(parent, tr("Open Profile"),
                                                  QString(), tr("Mudder profiles (*.mp);;All files (*)")));
    if (fileName.isEmpty())
    {
        return 0;
    }

    return openFile(fileName, parent);
}

Console * Console::openFile(const QString &fileName, QWidget *parent)
{
    Console *console = new Console(parent);
    if (console->readFile(fileName))
    {
        console->setCurrentFile(fileName);
        return console;
    }

    delete console;
    return 0;
}

void Console::closeEvent(QCloseEvent *e)
{
    if (okToContinue())
    {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

bool Console::okToContinue()
{
    if (isWindowModified())
    {
        QMessageBox::StandardButton answer = QMessageBox::warning(this, tr("Console"),
            tr("The profile has been modified.\nDo you want to save your changes?"),
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

bool Console::saveFile(const QString &fileName)
{
    if (writeFile(fileName))
    {
        setCurrentFile(fileName);
        return true;
    }

    return false;
}

void Console::setCurrentFile(const QString &fileName)
{
    m_fileName = fileName;
    m_isUntitled = false;

    m_action->setText(QFileInfo(fileName).fileName());

    setWindowTitle(QFileInfo(fileName).fileName() + "[*]");
    setWindowModified(false);
}

bool Console::readFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Console"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    CoreApplication::setApplicationBusy(true);

    LOG_INFO("Reading profile:", fileName);

    // TODO: read profile

    CoreApplication::setApplicationBusy(false);
    return true;
}

bool Console::writeFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Console"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    CoreApplication::setApplicationBusy(true);

    LOG_INFO("Writing profile:", fileName);

    // TODO: write profile

    CoreApplication::setApplicationBusy(false);
    return true;
}
