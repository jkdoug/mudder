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
#include "consoledocument.h"
#include "profile.h"
#include "xmlerror.h"
#include <QAbstractTextDocumentLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>

Console::Console(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Console)
{
    LOG_DEBUG("Setting up a new console window.");

    ui->setupUi(this);

    m_action = new QAction(this);
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(triggered()), SLOT(show()));
    connect(m_action, SIGNAL(triggered()), SLOT(setFocus()));

    m_isUntitled = true;

    m_document = new ConsoleDocument(this);
    ui->output->setDocument(m_document);

    connect(ui->input, SIGNAL(command(QString)), SLOT(commandEntered(QString)));

    QByteArray test("Rapture Runtime Environment v2.2.0 -- (c) 2012 -- Iron Realms Entertainment\n"
                    "Multi-User License: 100-0000-000\n"
                    "\n"
                    "[0;37m[33m   o0==============================~o[0]o~==============================0o\n"
                    "    IP Address:[35m 69.65.42.86[33m              Questions: [35msupport@lusternia.com\n"
                    "    [33mCurrently On-Line: [35m52\n"
                    "\n"
                    "[1;35m     .____                     __                         .__\n"
                    "     |    |     __ __  _______/  |_  _____ _______  ____  |__|_____\n"
                    "     |    |    |  |  \\/  ___/\\   __\\/  __ \\\\_  __ \\/    \\ |  |\\__  \\\n"
                    "     |    |___ |  |  /\\___ \\  |  |  \\  ___/ |  | \\/|  |  \\|  | / __ \\\n"
                    "     |_______ \\|____/ /____ \\ |__|   \\___ \\ |__|   |__|  /|__|/_____ \\\n"
                    "             \\/            \\/            \\/            \\/           \\/\n"
                    "\n"
                    "[0;35m                        A G E  O F  A S C E N S I O N\n"
                    "\n"
                    "\n"
                    "[33m   o0===================================================================0o\n"
                    "\n"
                    "[37m                 [35m1.[37m Enter the game.\n"
                    "                 [35m2.[37m Create a new character.\n"
                    "                 [35m3.[37m Quit.\n"
                    "\n"
                    "Enter an option or enter your character's name. \xFF\xFF");
    m_document->process(test);

    m_profile = new Profile(this);
    connect(m_profile, SIGNAL(optionsChanged()), SLOT(contentsModified()));
    connect(m_profile, SIGNAL(settingsChanged()), SLOT(contentsModified()));

    m_echoOn = true;
    m_disconnecting = false;

    m_connection = new Connection(this);
    connect(m_connection, SIGNAL(dataReceived(QByteArray)), SLOT(dataReceived(QByteArray)));
    connect(m_connection, SIGNAL(connected()), SLOT(connectionEstablished()));
    connect(m_connection, SIGNAL(disconnected()), SLOT(connectionLost()));
    connect(m_connection, SIGNAL(hostFound(QHostInfo)), SLOT(lookupComplete(QHostInfo)));
    connect(m_connection, SIGNAL(echo(bool)), SLOT(echoToggled(bool)));
    connect(m_connection, SIGNAL(toggleGMCP(bool)), SLOT(gmcpToggled(bool)));

    setWindowTitle("[*]");
    setAttribute(Qt::WA_DeleteOnClose);

    LOG_DEBUG("Console window initialized.");
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
    LOG_TRACE("Console::save", QString("Untitled: %1").arg(m_isUntitled), m_fileName);

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
    LOG_TRACE("Console::saveAs", fileName);
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
    LOG_TRACE("Console::open", fileName);
    if (fileName.isEmpty())
    {
        return 0;
    }

    return openFile(fileName, parent);
}

Console * Console::openFile(const QString &fileName, QWidget *parent)
{
    LOG_TRACE("Console::openFile", fileName);

    Console *console = new Console(parent);
    if (console->readFile(fileName))
    {
        console->setCurrentFile(fileName);
        return console;
    }

    delete console;
    return 0;
}

void Console::connectToServer()
{
    m_disconnecting = false;

    m_connection->connectRemote("lusternia.game-host.org", 23); // TODO
}

void Console::disconnectFromServer()
{
    m_disconnecting = true;

    m_connection->disconnectRemote();
}

void Console::closeEvent(QCloseEvent *e)
{
    LOG_TRACE("Console::closeEvent");

    if (okToContinue())
    {
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void Console::contentsModified()
{
    LOG_TRACE("Console::contentsModified");

    setWindowModified(true);
}

void Console::commandEntered(const QString &cmd)
{
    LOG_TRACE("Console::commandEntered", cmd);
}

void Console::connectionEstablished()
{
    LOG_INFO(tr("Connection established."));

    emit connectionStatusChanged(true);
}

void Console::connectionLost()
{
    quint64 duration = m_connection->connectDuration();
//    colorNote("dimgray", QColor(), QString(20, QLatin1Char('-')));
    LOG_INFO(tr("Disconnected from server. Total time connected: %1:%2:%3.%4")
             .arg((duration / (60 * 60 * 1000)) % 60)
             .arg((duration / (60 * 1000)) % 60, 2, 10, QLatin1Char('0'))
             .arg((duration / 1000) % 60, 2, 10, QLatin1Char('0'))
             .arg(duration % 1000, 3, 10, QLatin1Char('0')));

    emit connectionStatusChanged(false);
}

void Console::lookupComplete(const QHostInfo &hostInfo)
{
    QString address;
    if (!hostInfo.addresses().isEmpty())
    {
        address = hostInfo.addresses().first().toString();
    }
    else
    {
        address = hostInfo.hostName();
    }
    LOG_INFO(tr("Host lookup answer: %1").arg(address));
}

void Console::dataReceived(const QByteArray &data)
{
//    timeLatency(m_connection->latency() / 1000.0);

//    QElapsedTimer processingTime;
//    processingTime.start();

    m_document->process(data);

//    timeProcessing(processingTime.elapsed() / 1000.0);

//    updateScroll();

//    logLatest();
}

void Console::echoToggled(bool on)
{
    m_echoOn = on;
}

void Console::gmcpToggled(bool on)
{
    if (on)
    {
        LOG_INFO(tr("GMCP enabled."));
    }
    else
    {
        LOG_INFO(tr("GMCP disabled."));
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
    LOG_TRACE("Console::saveFile", fileName);

    if (writeFile(fileName))
    {
        setCurrentFile(fileName);
        return true;
    }

    return false;
}

void Console::setCurrentFile(const QString &fileName)
{
    LOG_TRACE("Console::setCurrentFile", fileName);

    m_fileName = fileName;
    m_isUntitled = false;

    m_action->setText(QFileInfo(fileName).fileName());

    setWindowTitle(QFileInfo(fileName).fileName() + "[*]");
    setWindowModified(false);
}

bool Console::readFile(const QString &fileName)
{
    LOG_TRACE("Console::readFile", fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Console"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));

        LOG_WARNING("Cannot read profile:", fileName, file.errorString());
        return false;
    }

    CoreApplication::setApplicationBusy(true);

    LOG_INFO("Reading profile:", fileName);

    QXmlStreamReader xml(&file);

    QList<XmlError *> errors;
    m_profile->fromXml(xml, errors);

    file.close();

    CoreApplication::setApplicationBusy(false);

    foreach (XmlError *err, errors)
    {
        LOG_WARNING(err->toString());

        // TODO: append to console document

        delete err;
    }
    errors.clear();

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

        LOG_WARNING("Cannot write profile:", fileName, file.errorString());
        return false;
    }

    CoreApplication::setApplicationBusy(true);

    LOG_INFO("Writing profile:", fileName);

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);
    m_profile->toXml(xml);
    file.close();

    CoreApplication::setApplicationBusy(false);

    return true;
}
