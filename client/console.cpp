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


#include "console.h"
#include "ui_console.h"
#include "coreapplication.h"
#include "consoledocument.h"
#include "engine.h"
#include "logging.h"
#include "profile.h"
#include "xmlerror.h"
#include "accelerator.h"
#include "alias.h"
#include "event.h"
#include "group.h"
#include "timer.h"
#include "trigger.h"
#include <QAbstractTextDocumentLayout>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QTextDocumentFragment>
#include <QToolTip>

Console::Console(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Console)
{
    qCDebug(MUDDER_CONSOLE) << "Setting up a new console window.";

    ui->setupUi(this);

    m_action = new QAction(this);
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(triggered()), SLOT(show()));
    connect(m_action, SIGNAL(triggered()), SLOT(setFocus()));

    m_isUntitled = true;

    connect(ui->input, SIGNAL(command(QString)), SLOT(commandEntered(QString)));
    connect(ui->input, SIGNAL(script(QString)), SLOT(scriptEntered(QString)));

    m_profile = new Profile(this);
    connect(m_profile, SIGNAL(optionChanged(QString, QVariant)), SLOT(contentsModified()));
    connect(m_profile, SIGNAL(optionChanged(QString, QVariant)), ui->input, SLOT(optionChanged(QString, QVariant)));
    connect(m_profile, SIGNAL(settingsChanged()), SLOT(contentsModified()));
    connect(m_profile, SIGNAL(timerFired(Timer*)), SLOT(processTimer(Timer*)));

    m_document = new ConsoleDocument(this);
    m_document->setMaximumBlockCount(m_profile->scrollbackLines() + 1);
    ui->output->setDocument(m_document);
    connect(m_profile, SIGNAL(optionChanged(QString, QVariant)), m_document, SLOT(optionChanged(QString, QVariant)));

    m_echoOn = true;

    m_mousePressed = false;
    m_selectionStart = 0;
    m_selectionEnd = 0;
    m_clickPos = -1;

    m_connection = new Connection(this);
    connect(m_connection, SIGNAL(dataReceived(QByteArray)), SLOT(dataReceived(QByteArray)));
    connect(m_connection, SIGNAL(connected()), SLOT(connectionEstablished()));
    connect(m_connection, SIGNAL(disconnected()), SLOT(connectionLost()));
    connect(m_connection, SIGNAL(hostFound(QHostInfo)), SLOT(lookupComplete(QHostInfo)));
    connect(m_connection, SIGNAL(echo(bool)), SLOT(echoToggled(bool)));
    connect(m_connection, SIGNAL(echo(bool)), ui->input, SLOT(echoToggled(bool)));

    connect(ui->input, SIGNAL(accelerator(QKeySequence)), SLOT(processAccelerators(QKeySequence)));
    connect(m_document, SIGNAL(blockAdded(QTextBlock, bool)), SLOT(processTriggers(QTextBlock, bool)));
    connect(m_document, SIGNAL(contentsChanged()), ui->output, SLOT(update()));
    connect(m_document, SIGNAL(contentsChanged()), SLOT(updateScroll()));
    connect(ui->scrollbar, SIGNAL(valueChanged(int)), SLOT(scrollbarMoved(int)));

    m_engine = new Engine(this);
    m_engine->initialize(this);
    connect(m_connection, SIGNAL(toggleGMCP(bool)), m_engine, SLOT(enableGMCP(bool)));
    connect(m_connection, SIGNAL(receivedGMCP(QString, QString)), m_engine, SLOT(handleGMCP(QString, QString)));

    setWindowTitle("[*]");
    setAttribute(Qt::WA_DeleteOnClose);

    qCDebug(MUDDER_CONSOLE) << "Console window initialized.";
    printInfo("Hello, Fodder!");
}

Console::~Console()
{
    delete ui;
}

void Console::newFile()
{
    static int fileNumber = 1;

    m_fileName = tr("profile%1").arg(fileNumber++);
    m_profile->setName(m_fileName);
    setWindowTitle(m_fileName + "[*]");
    m_action->setText(m_fileName);
    m_isUntitled = true;

    qCDebug(MUDDER_PROFILE) << "Created a new profile:" << m_fileName;
}

bool Console::save()
{
    qCDebug(MUDDER_PROFILE) << "Save: filename =" << m_fileName << "untitled =" << m_isUntitled;

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

    qCDebug(MUDDER_PROFILE) << "SaveAs: filename =" << fileName;

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

    qCDebug(MUDDER_PROFILE) << "Open: filename =" << fileName;

    if (fileName.isEmpty())
    {
        return 0;
    }

    return openFile(fileName, parent);
}

Console * Console::openFile(const QString &fileName, QWidget *parent)
{
    qCDebug(MUDDER_PROFILE) << "OpenFile: filename =" << fileName;

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
    m_connection->connectRemote(m_profile->address(), m_profile->port());
}

void Console::disconnectFromServer()
{
    m_connection->disconnectRemote();
}

void Console::scrollUp(int lines)
{
    scrollTo(ui->scrollbar->value() - lines);
}

void Console::scrollDown(int lines)
{
    scrollTo(ui->scrollbar->value() + lines);
}

void Console::scrollTo(int line)
{
    line = qBound(ui->scrollbar->minimum(), line, ui->scrollbar->maximum());

    ui->scrollbar->setValue(line);
    ui->output->setScrollLines(line);
    ui->output->update();
}

void Console::scrollToTop()
{
    scrollTo(ui->scrollbar->minimum());
}

void Console::scrollToBottom()
{
    scrollTo(ui->scrollbar->maximum());
}

void Console::printInfo(const QString &msg)
{
    m_document->info(msg);
    scrollToBottom();
}

void Console::printWarning(const QString &msg)
{
    m_document->warning(msg);
    scrollToBottom();
}

void Console::printError(const QString &msg)
{
    m_document->error(msg);
    scrollToBottom();
}

bool Console::send(const QString &cmd, bool show)
{
    bool result = m_connection->send(cmd);
    if (show)
    {
        m_document->command(cmd);
        scrollToBottom();
    }
    return result;
}

bool Console::sendAlias(const QString &cmd)
{
    commandEntered(cmd);

    return true;
}

bool Console::sendGmcp(const QString &msg, const QString &data)
{
    return m_connection->sendGmcp(msg, data);
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

void Console::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_document)
    {
        return;
    }

    if (m_mousePressed)
    {
        QPointF hitPos(e->pos().x(), fabs(e->pos().y() - ui->output->height()));

        int pos = m_document->documentLayout()->hitTest(hitPos, Qt::FuzzyHit);
        if (pos < 0)
        {
            e->ignore();
            return;
        }

        m_selectionEnd = pos;

        m_document->select(m_selectionStart, m_selectionEnd);

        if (e->y() < 10)
        {
            scrollUp(1);
        }
        else if (e->y() > ui->output->height() - 10)
        {
            scrollDown(1);
        }
    }

    e->accept();
}

void Console::mousePressEvent(QMouseEvent *e)
{
    if (!m_document)
    {
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        QPointF hitPos(e->pos().x(), fabs(e->pos().y() - ui->output->height()));

        m_clickPos = m_document->documentLayout()->hitTest(hitPos, Qt::ExactHit);

        m_mousePressed = true;
        m_selectionStart = m_clickPos;
        m_selectionEnd = m_clickPos;
    }
}

void Console::mouseReleaseEvent(QMouseEvent *e)
{
    if (!m_document)
    {
        return;
    }

    if (e->button() == Qt::RightButton)
    {
        QMenu *popup = new QMenu(this);

        if (m_document->hasSelection())
        {
            QAction *actionCopy = new QAction(tr("&Copy"), this);
            actionCopy->setStatusTip(tr("Copy selected text to clipboard"));
            connect(actionCopy, SIGNAL(triggered()), SLOT(copy()));

            QAction *actionCopyHtml = new QAction(tr("Copy as &HTML"), this);
            actionCopyHtml->setStatusTip(tr("Copy selected text to clipboard as HTML"));
            connect(actionCopyHtml, SIGNAL(triggered()), SLOT(copyHtml()));

            QAction *actionSelectNone = new QAction(tr("Select &None"), this);
            actionSelectNone->setStatusTip(tr("Removes the current text selection"));
            connect(actionSelectNone, SIGNAL(triggered()), m_document, SLOT(selectNone()));

            popup->addAction(actionCopy);
            popup->addAction(actionCopyHtml);
            popup->addSeparator();
            popup->addAction(actionSelectNone);
        }

        QAction *actionSelectAll = new QAction(tr("Select &All"), this);
        actionSelectAll->setStatusTip(tr("Select all buffered output text"));
        connect(actionSelectAll, SIGNAL(triggered()), m_document, SLOT(selectAll()));

        QAction *actionClearBuffer = new QAction(tr("C&lear output buffer"), this);
        actionClearBuffer->setStatusTip(tr("Deletes all text stored in the output text buffer"));
        connect(actionClearBuffer, SIGNAL(triggered()), m_document, SLOT(clear()));

        popup->addAction(actionSelectAll);
        popup->addSeparator();
        popup->addAction(actionClearBuffer);

        popup->popup(mapToGlobal(e->pos()), popup->actions().at(0));
    }
    else if (e->button() == Qt::MiddleButton)
    {
        scrollTo(m_document->blockCount());
    }
    else if (e->button() == Qt::LeftButton)
    {
        // TODO: don't clear selection when clicking within the selected text?
        QPointF hitPos(e->pos().x(), fabs(e->pos().y() - ui->output->height()));

        if (m_document->documentLayout()->hitTest(hitPos, Qt::ExactHit) == m_clickPos)
        {
            m_document->selectNone();

            m_selectionStart = 0;
            m_selectionEnd = 0;
        }

        m_mousePressed = false;
        m_clickPos = -1;
    }
    else
    {
        QString anchor(m_document->documentLayout()->anchorAt(e->pos()));
        if (anchor != m_linkHovered)
        {

            m_linkHovered = anchor;

            if (m_linkHovered.isEmpty())
            {
                QToolTip::hideText();
                setCursor(Qt::IBeamCursor);
            }
            else
            {
                QToolTip::showText(e->globalPos(), "Test hint\nhttp://google.com");
                setCursor(Qt::PointingHandCursor);
            }
        }
    }

    e->accept();
}

void Console::wheelEvent(QWheelEvent *e)
{
    const int k = 3;
    if (e->delta() < 0)
    {
        scrollDown(k);
    }
    else if (e->delta() > 0)
    {
        scrollUp(k);
    }
    e->accept();
}


void Console::processAccelerators(const QKeySequence &key)
{
    QList<Accelerator *> accelerators(m_profile->rootGroup()->sortedAccelerators());
    foreach (Accelerator *accelerator, accelerators)
    {
        if (accelerator->key() == key)
        {
            Group *previousGroup = m_profile->activeGroup();
            Q_ASSERT(previousGroup != 0);
            m_profile->setActiveGroup(accelerator->group());

            if (!accelerator->execute(m_engine))
            {
                m_profile->setActiveGroup(previousGroup);
                accelerator->setFailed(true);
                continue;
            }

            m_profile->setActiveGroup(previousGroup);

            ui->input->setAccelerated(true);
            break;
        }
    }
}

void Console::processAliases(const QString &cmd)
{
    bool matched = false;
    QList<Alias *> aliases(m_profile->rootGroup()->sortedAliases());
    foreach (Alias *alias, aliases)
    {
        if (!alias->regex().isValid())
        {
            printError(tr("Alias %1: %2 (column %3)").arg(alias->name()).arg(alias->regex().errorString()).arg(alias->regex().patternErrorOffset()));
            alias->setFailed(true);
            continue;
        }

        if (alias->match(cmd))
        {
            Group *previousGroup = m_profile->activeGroup();
            Q_ASSERT(previousGroup != 0);
            m_profile->setActiveGroup(alias->group());

            if (!alias->execute(m_engine))
            {
                m_profile->setActiveGroup(previousGroup);
                alias->setFailed(true);

                if (!alias->keepEvaluating())
                {
                    return;
                }

                continue;
            }

            m_profile->setActiveGroup(previousGroup);

            if (!alias->keepEvaluating())
            {
                return;
            }

            matched = true;
        }
    }

    if (!matched)
    {
        m_connection->send(cmd);

        if (m_echoOn)
        {
            m_document->command(cmd);
        }
    }
}

void Console::processEvents(const QString &name, const QVariantList &args)
{
    QList<Event *> events(m_profile->rootGroup()->sortedEvents());
    foreach (Event *event, events)
    {
        if (event->match(name))
        {
            Group *previousGroup = m_profile->activeGroup();
            Q_ASSERT(previousGroup != 0);
            m_profile->setActiveGroup(event->group());

            if (!event->execute(m_engine, args))
            {
                m_profile->setActiveGroup(previousGroup);
                event->setFailed(true);

                continue;
            }

            m_profile->setActiveGroup(previousGroup);
        }
    }
}

void Console::processTriggers(QTextBlock block, bool prompt)
{
    Q_UNUSED(prompt)

    bool omitted = false;
    QString text(block.text());
    QList<Trigger *> triggers(m_profile->rootGroup()->sortedTriggers());
    foreach (Trigger *trigger, triggers)
    {
        bool matched = false;
        bool keepEvaluating = trigger->keepEvaluating();
        int pos = 0;

        while (pos < text.length())
        {
            if (!trigger->regex().isValid())
            {
                printError(tr("Trigger %1: %2 (column %3)").arg(trigger->name()).arg(trigger->regex().errorString()).arg(trigger->regex().patternErrorOffset()));
                trigger->setFailed(true);
                break;
            }

            matched = trigger->match(text, pos);
            if (matched)
            {
                Group *previousGroup = m_profile->activeGroup();
                Q_ASSERT(previousGroup != 0);
                m_profile->setActiveGroup(trigger->group());

                if (!trigger->execute(m_engine))
                {
                    m_profile->setActiveGroup(previousGroup);
                    trigger->setFailed(true);
                    keepEvaluating = false;
                    break;
                }

                m_profile->setActiveGroup(previousGroup);

                if (!omitted && trigger->omit())
                {
                    omitted = true;
                }
            }

            if (!trigger->repeat())
            {
                break;
            }

            pos = trigger->matchEnd();
        }

        if (matched && !keepEvaluating)
        {
            break;
        }
    }

    // TODO: check for omitted lines to delete
}

void Console::processTimer(Timer *timer)
{
    if (!timer)
    {
        return;
    }

    qCDebug(MUDDER_SCRIPT) << "Timer fired:" << timer->fullName();

    if (!timer->execute(m_engine))
    {
        timer->enable(false);
    }

    if (timer->once())
    {
        m_profile->deleteItem(m_profile->indexForPath(timer->path()));
    }
}

void Console::contentsModified()
{
    setWindowModified(true);
    emit modified();
}

void Console::commandEntered(const QString &cmd)
{
    qCDebug(MUDDER_CONSOLE) << "Command entered:" << cmd;

    // TODO: re-design this to push functionality to the command line
    //  and signal commands to be processed instead

    if (cmd.isEmpty())
    {
        send("");
    }
    else
    {
        processAliases(cmd);
    }

    scrollToBottom();
}

void Console::scriptEntered(const QString &code)
{
    qCDebug(MUDDER_CONSOLE) << "Script entered:" << code;

    m_engine->execute(code);
}

void Console::connectionEstablished()
{
    qCDebug(MUDDER_NETWORK) << "Connection established.";

    printInfo(tr("Connected to %1:%2.").arg(m_profile->address()).arg(m_profile->port()));

    emit connectionStatusChanged(true);
}

void Console::connectionLost()
{
    qCDebug(MUDDER_NETWORK) << "Connection lost.";

    quint64 duration = m_connection->connectDuration();
    printInfo(tr("Disconnected from server."));
    printInfo(tr("Total time connected: %1:%2:%3.%4")
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

    qCDebug(MUDDER_NETWORK) << "Host lookup result:" << address;
}

void Console::dataReceived(const QByteArray &data)
{
    m_document->process(data);

    updateScroll();
    scrollToBottom();
}

void Console::echoToggled(bool on)
{
    m_echoOn = on;
}

void Console::scrollbarMoved(int pos)
{
    disconnect(ui->scrollbar, SIGNAL(valueChanged(int)), this, SLOT(scrollbarMoved(int)));

    scrollTo(pos);

    connect(ui->scrollbar, SIGNAL(valueChanged(int)), SLOT(scrollbarMoved(int)));
}

void Console::updateScroll()
{
    if (ui->output->documentLayout())
    {
        ui->scrollbar->setRange(1, ui->output->documentLayout()->documentSize().height());
    }
    else
    {
        ui->scrollbar->setRange(1, 1);
    }
    ui->scrollbar->setPageStep(10);
}

void Console::copy()
{
    QTextDocumentFragment fragment(*m_document->cursor());
    QApplication::clipboard()->setText(fragment.toPlainText());

    m_document->selectNone();
}

void Console::copyHtml()
{
    // TODO: much more efficient toHtml
    QTextDocumentFragment fragment(*m_document->cursor());
    QApplication::clipboard()->setText(fragment.toHtml("utf-8"));

    m_document->selectNone();
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
    qCDebug(MUDDER_PROFILE) << "SaveFile: filename =" << fileName;

    if (writeFile(fileName))
    {
        setCurrentFile(fileName);
        return true;
    }

    return false;
}

void Console::setCurrentFile(const QString &fileName)
{
    qCDebug(MUDDER_PROFILE) << "CurrentFile: filename =" << fileName;

    m_fileName = fileName;
    m_isUntitled = false;

    m_action->setText(QFileInfo(fileName).fileName());

    if (m_profile->name().isEmpty())
    {
        m_profile->setName(QFileInfo(fileName).fileName());
    }

    setWindowTitle(m_profile->name() + "[*]");
    setWindowModified(false);
}

bool Console::readFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCWarning(MUDDER_PROFILE) << "Cannot read:" << fileName << file.errorString();

        QMessageBox::warning(this, tr("Console"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    CoreApplication::setApplicationBusy(true);

    qCDebug(MUDDER_PROFILE) << "Reading:" << fileName;

    QXmlStreamReader xml(&file);

    QList<XmlError *> errors;
    m_profile->fromXml(xml, errors);

    file.close();

    CoreApplication::setApplicationBusy(false);

    foreach (XmlError *err, errors)
    {
        qCWarning(MUDDER_PROFILE) << *err;

        m_document->warning(err->toString());

        delete err;
    }
    errors.clear();

    scrollToBottom();

//    Trigger *trigTest = new Trigger;
//    trigTest->setPattern("\\bE(nte)r\\b");
//    trigTest->setName("trigTest");
//    trigTest->setContents("print('hello world')");
//    m_profile->rootGroup()->addItem(trigTest);

//    QByteArray test("Rapture Runtime Environment v2.2.0 -- (c) 2012 -- Iron Realms Entertainment\n"
//                    "Multi-User License: 100-0000-000\n"
//                    "\n"
//                    "[0;37m[33m   o0==============================~o[0]o~==============================0o\n"
//                    "    IP Address:[35m 69.65.42.86[33m              Questions: [35msupport@lusternia.com\n"
//                    "    [33mCurrently On-Line: [35m52\n"
//                    "\n"
//                    "[1;35m     .____                     __                         .__\n"
//                    "     |    |     __ __  _______/  |_  _____ _______  ____  |__|_____\n"
//                    "     |    |    |  |  \\/  ___/\\   __\\/  __ \\\\_  __ \\/    \\ |  |\\__  \\\n"
//                    "     |    |___ |  |  /\\___ \\  |  |  \\  ___/ |  | \\/|  |  \\|  | / __ \\\n"
//                    "     |_______ \\|____/ /____ \\ |__|   \\___ \\ |__|   |__|  /|__|/_____ \\\n"
//                    "             \\/            \\/            \\/            \\/           \\/\n"
//                    "\n"
//                    "[0;35m                        A G E  O F  A S C E N S I O N\n"
//                    "\n"
//                    "\n"
//                    "[33m   o0===================================================================0o\n"
//                    "\n"
//                    "[37m                 [35m1.[37m Enter the game.\n"
//                    "                 [35m2.[37m Create a new character.\n"
//                    "                 [35m3.[37m Quit.\n"
//                    "\n"
//                    "Enter an option or enter your character's name. \xFF");
//    dataReceived(test);

    return true;
}

bool Console::writeFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCWarning(MUDDER_PROFILE) << "Cannot write:" << fileName << file.errorString();

        QMessageBox::warning(this, tr("Console"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    CoreApplication::setApplicationBusy(true);

    qCDebug(MUDDER_PROFILE) << "Writing:" << fileName;

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);
    m_profile->toXml(xml);
    file.close();

    CoreApplication::setApplicationBusy(false);

    return true;
}
