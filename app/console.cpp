/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "console.h"
#include "ui_console.h"
#include "commandline.h"
#include "consoledocumentlayout.h"
#include "engine.h"
#include "event.h"
#include "mapengine.h"
#include "profile.h"
#include "textblockdata.h"
#include "textdocument.h"
#include "xmlexception.h"
#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMenu>
#include <QMessageBox>
#include <QTextBlock>
#include <QToolTip>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

Console::Console(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Console)
{
    m_connection = new Connection(this);
    m_document = new TextDocument(this);
    m_profile = new Profile(this);
    m_engine = new Engine(this);

    m_preferences = 0;
    m_settings = 0;

    m_logPosition = 0;
    m_logPaused = false;

    ui->setupUi(this);

    m_media = new QMediaPlayer(this);

    m_maxPackets = 50;

    m_formatSelection.setForeground(QColor("gainsboro"));
    m_formatSelection.setBackground(QColor("dodgerblue"));

    m_mousePressed = false;
    m_selectionStart = 0;
    m_selectionEnd = 0;
    m_clickPos = -1;

    m_document->setMaximumBlockCount(1000);

    m_document->setFont(profile()->outputFont());
    ui->input->setFont(profile()->inputFont());

    ui->scrollback->hide();

    updateScroll();

    m_document->setDocumentLayout(new ConsoleDocumentLayout(m_document));

    ui->scrollback->setDocument(m_document);
    ui->output->setDocument(m_document);

    ui->scrollback->setConsole(this);
    ui->output->setConsole(this);
    ui->input->setConsole(this);

    ui->input->setFocus();

    setWindowTitle(profile()->name());
    setEchoOn(true);

    m_disconnecting = false;

    connect(ui->scrollbar, SIGNAL(valueChanged(int)), this, SLOT(scrollbarMoved(int)));

    connect(ui->input, SIGNAL(command(QString)), this, SLOT(commandEntered(QString)));
    connect(ui->input, SIGNAL(script(QString)), this, SLOT(scriptEntered(QString)));

    connect(connection(), SIGNAL(dataReceived(QByteArray)), this, SLOT(dataReceived(QByteArray)));
    connect(connection(), SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(connection(), SIGNAL(disconnected()), this, SLOT(connectionLost()));
    connect(connection(), SIGNAL(hostFound(QHostInfo)), this, SLOT(lookupComplete(QHostInfo)));
    connect(connection(), SIGNAL(echo(bool)), this, SLOT(echoToggled(bool)));
    connect(connection(), SIGNAL(toggleGMCP(bool)), this, SLOT(gmcpToggled(bool)));

    connect(profile(), SIGNAL(backgroundChanged()), this, SLOT(backgroundChanged()));
    connect(profile(), SIGNAL(inputFontChanged()), this, SLOT(inputFontChanged()));
    connect(profile(), SIGNAL(outputFontChanged()), this, SLOT(outputFontChanged()));

    connect(document(), SIGNAL(lineAdded(QTextBlock)), this, SLOT(lineAdded(QTextBlock)));
    connect(document(), SIGNAL(contentsChanged()), ui->output, SLOT(update()));
    connect(document(), SIGNAL(contentsChanged()), ui->scrollback, SLOT(update()));
    connect(document(), SIGNAL(contentsChanged()), this, SLOT(updateScroll()));
}

Console::~Console()
{
    if (isLogging())
    {
        finishLog();
    }

    delete ui;
}

DialogProfile *Console::preferences()
{
    if (m_preferences == 0)
    {
        m_preferences = new DialogProfile(this);
    }

    m_preferences->load(profile());
    return m_preferences;
}

DialogSettings *Console::settings()
{
    if (m_settings == 0)
    {
        m_settings = new DialogSettings(this);
    }

    m_settings->load(profile());
    return m_settings;
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
    if (!ui->scrollback->isVisible())
    {
        ui->scrollbar->setValue(ui->scrollbar->maximum());
    }
}

void Console::logLatest()
{
    if (isLogging())
    {
        int len = document()->characterCount() - 1;

        logNote(document()->toHtml(m_logPosition, len, Qt::lightGray, profile()->background(), profile()->outputFont()));

        m_logPosition = len;
    }
}

void Console::connectionEstablished()
{
    systemInfo(tr("Connection established"));
}

void Console::connectionLost()
{
    systemInfo(tr("Disconnected"));
    quint64 duration = connection()->connectDuration();
    systemInfo(QString(tr("Total time connected: %1:%2:%3.%4"))
              .arg((duration / (60 * 60 * 1000)) % 60)
              .arg((duration / (60 * 1000)) % 60, 2, 10, QLatin1Char('0'))
              .arg((duration / 1000) % 60, 2, 10, QLatin1Char('0'))
              .arg(duration % 1000, 3, 10, QLatin1Char('0')));
    colorNote("dimgray", QColor(), QString(20, QLatin1Char('-')));

    if (!m_disconnecting && profile()->autoReconnect())
    {
        connectToServer();
    }
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
    systemInfo(tr("Host lookup = %1").arg(address));
}


void Console::dataReceived(const QByteArray &data)
{
    timeLatency(connection()->latency() / 1000.0);

    QElapsedTimer processingTime;
    processingTime.start();

    document()->process(data);

    timeProcessing(processingTime.elapsed() / 1000.0);

    updateScroll();

    ui->latency->setText(QString("L: %1\nP: %2").arg(meanLatency(), 0, 'f', 3).arg(meanProcessing(), 0, 'f', 3));

    logLatest();
}

void Console::echoToggled(bool on)
{
    m_echoOn = on;
}

void Console::gmcpToggled(bool on)
{
    if (on)
    {
        systemInfo(tr("GMCP enabled"));
    }
    else
    {
        systemInfo(tr("GMCP disabled"));
    }
}

void Console::handleGMCP(const QString &name, const QString &data)
{
    if (!engine())
    {
        return;
    }


    QVariantList args;
    args << name;

    if (!data.isEmpty())
    {
        if (data.at(0) == QLatin1Char('\'') || data.at(0) == QLatin1Char('"'))
        {
            args << data;
        }
        else
        {
            QJsonParseError error;
            QJsonDocument doc(QJsonDocument::fromJson(data.toUtf8(), &error));
            if (error.error != QJsonParseError::NoError)
            {
                qDebug() << tr("GMCP mismatch on '%1' '%2'; invalid JSON data at column %3: %4").arg(name).arg(data).arg(error.offset).arg(error.errorString());
                return;
            }

            args << doc;
        }
    }

    engine()->processEvent("onGMCP", args);
}

void Console::handleMouseEvent(QMouseEvent *e, const QPoint &pos)
{
    if (!engine())
    {
        return;
    }

    QString evt("onMouse");
    switch (e->type())
    {
        case QEvent::MouseButtonPress:
        {
            evt = "onMousePress";
        }
        break;

        case QEvent::MouseButtonRelease:
        {
            evt = "onMouseRelease";
        }
        break;

        case QEvent::MouseButtonDblClick:
        {
            evt = "onMouseDoubleClick";
        }
        break;

        case QEvent::MouseMove:
        {
            evt = "onMouseMove";
        }
        break;
    }

    engine()->processEvent(evt, QVariantList() << e->button() << e->pos() << pos << (int)e->modifiers());
}

void Console::scrollbarMoved(int val)
{
    disconnect(ui->scrollbar, SIGNAL(valueChanged(int)), this, SLOT(scrollbarMoved(int)));

    scrollTo(val);

    connect(ui->scrollbar, SIGNAL(valueChanged(int)), this, SLOT(scrollbarMoved(int)));
}

void Console::lineAdded(const QTextBlock &block)
{
    if (!processTriggers(block.text()))
    {
        // TODO: hide/delete the block
    }
}

void Console::commandEntered(const QString &cmd)
{
    if (!processAliases(cmd))
    {
        send(cmd, echoOn());
    }

    ui->latency->setText(QString("L: %1\nP: %2").arg(meanLatency(), 0, 'f', 3).arg(meanProcessing(), 0, 'f', 3));

    if (!echoOn())
    {
        ui->input->clear();
    }
}

void Console::scriptEntered(const QString &code)
{
    engine()->execute(code);
}

void Console::connectToServer()
{
    m_disconnecting = false;

    connection()->connectRemote(profile()->address(), profile()->port());
}

void Console::disconnectFromServer()
{
    m_disconnecting = true;

    connection()->disconnectRemote();
}

void Console::startEngine(bool restart)
{
    if (!restart)
    {
        colorNote("limegreen", QColor(), tr("Hello, Fodder! This is Mudder %1").arg(QApplication::applicationVersion()));
        hyperlink("Test link\n", Qt::blue, QColor(), new Hyperlink("Click here to test", QUrl(QString("alias://foo")), this));
    }

    QString script;
    if (!profile()->scriptFilename().isEmpty())
    {
        QFile *file = new QFile(profile()->scriptFilename(), this);
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            systemWarn(QString("Failed to open script: %1").arg(profile()->scriptFilename()));
            return;
        }

        script = file->readAll();
    }

    engine()->initialize(this, script);

    connect(connection(), SIGNAL(receivedGMCP(QString,QString)), this, SLOT(handleGMCP(QString,QString)));

    profile()->setEngine(engine());
    profile()->initTimers();
}

bool Console::loadProfile(const QString &filename)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QApplication::restoreOverrideCursor();
        return false;
    }

    try
    {
        QXmlStreamReader xml(&file);
        profile()->fromXml(xml);
    }
    catch (XmlException *xe)
    {
        foreach (QString warning, xe->warnings())
        {
            systemWarn(warning);
        }

        delete xe;
    }

    profile()->setFilename(filename);

    if (profile()->name().isEmpty())
    {
        profile()->setName(QFileInfo(filename).fileName());
        profile()->setDirty(true);
    }
    else
    {
        profile()->setDirty(false);
    }

    document()->setFont(profile()->outputFont());
    document()->setCommandColor(profile()->commandForeground(), profile()->commandBackground());

    QApplication::restoreOverrideCursor();
    return true;
}

bool Console::saveProfile(const QString &filename)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QApplication::restoreOverrideCursor();
        return false;
    }

    try
    {
        QXmlStreamWriter xml(&file);
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(2);
        profile()->toXml(xml);

        if (xml.hasError())
        {
            throw new XmlException("Unspecified error occurred while formatting the XML to save profile");
        }

        profile()->setFilename(filename);
        profile()->setDirty(false);
    }
    catch (XmlException *xe)
    {
        foreach (QString warning, xe->warnings())
        {
            systemWarn(warning);
        }

        delete xe;

        QApplication::restoreOverrideCursor();
        return false;
    }

    QApplication::restoreOverrideCursor();
    return true;
}

QFont Console::inputFont() const
{
    return ui->input->font();
}

void Console::setInputFont(const QFont &font)
{
    ui->input->setFont(font);

    ui->input->update();
}

QFont Console::outputFont() const
{
    return m_document->defaultFont();
}

void Console::setOutputFont(const QFont &font)
{
    document()->setFont(font);

    ui->output->update();
    ui->scrollback->update();
}

void Console::focusCommandLine()
{
    ui->input->setFocus();
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
    ui->scrollback->setScrollLines(line);

    if (line >= ui->scrollbar->maximum())
    {
        ui->scrollback->hide();
    }
    else
    {
        ui->scrollback->show();

        if (ui->scrollback->height() < 5)
        {
            ui->splitter->setSizes(QList<int>() << height() / 3 << height() / 3 * 2);
        }
        else
        {
            ui->scrollback->update();
        }
    }
}

QString Console::startLog(const QString &filename, bool append)
{
    QFileInfo logInfo;
    QString logFilename(filename);
    if (logFilename.isEmpty())
    {
        logFilename = profile()->logFilename();
        logInfo = QFileInfo(QDir(profile()->logDirectory()), logFilename);
    }

    logInfo.setCaching(false);

    if (logInfo.suffix().isEmpty())
    {
        logInfo.setFile(logInfo.fileName() + ".html");
    }

    if (!logInfo.dir().mkpath(logInfo.path()))
    {
        return tr("Failed to create log directory: %1").arg(logInfo.path());
    }

    logFilename.replace("%P", profile()->name());
    logFilename.replace("%D", QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    logFilename.replace("%T", QDateTime::currentDateTime().toString("hh-mm-ss"));
    logInfo.setFile(logInfo.dir(), logFilename);

    bool exists = logInfo.size() > 0;

    QFlags<QIODevice::OpenModeFlag> openMode(QIODevice::WriteOnly | QIODevice::Text | (append ? QIODevice::Append : QIODevice::Truncate));
    QFile *logFile = new QFile(logInfo.filePath(), this);
    if (!logFile->open(openMode))
    {
        delete logFile;
        return tr("Failed to open log file: %1").arg(logInfo.filePath());
    }

    log().setDevice(logFile);
    m_logInfo = logInfo;

    QString name(profile()->name());
    if (name.isEmpty())
    {
        name = profile()->filename();
        if (name.isEmpty())
        {
            name = "(Untitled)";
        }
    }

    if (append && exists)
    {
        log() << QString("\n<!-- Start: %1 -->\n")
                 .arg(QDateTime::currentDateTime().toString());
    }
    else
    {
        QFont font(profile()->outputFont());
        QColor fg(Qt::lightGray);
        QColor bg(profile()->background());
        log() << QString("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>\n"
                 "<html>\n"
                 " <head>\n"
                 "  <style>\n"
                 "  <!--\n"
                 "   body { font-family: '%1', 'Courier New', 'Monospace', 'Courier'; }\n"
                 "   body { font-size: %2px; }\n"
                 "   body { white-space: pre-wrap; }\n"
                 "   body { color: %3; }\n"
                 "   body { background-color: %4; }\n"
                 "  -->\n"
                 "  </style>\n"
                 "  <title>%5</title>\n"
                 "  <meta http-equiv='content-type' content='text/html; charset=utf-8'>\n"
                 " </head>\n"
                 "<!-- Start: %6 -->\n"
                 " <body>\n")
                 .arg(font.family())
                 .arg(font.pixelSize())
                 .arg(fg.name())
                 .arg(bg.name())
                 .arg(name)
                 .arg(QDateTime::currentDateTime().toString());
    }

    pauseLog(true);
    systemInfo(tr("Logging to %1").arg(m_logInfo.canonicalFilePath()));
    pauseLog(false);

    emit logOpened(logInfo.filePath());

    m_logPosition = document()->characterCount() - 1;

    return QString();
}

void Console::logNote(const QString &text)
{
    log() << text;
    log().flush();
}

void Console::finishLog()
{
    if (!isLogging())
    {
        return;
    }

    log() << QString("\n"
             "</body>\n"
             "<!-- Stop: %1 -->\n"
             "</html>")
             .arg(QDateTime::currentDateTime().toString());

    QIODevice *dev = log().device();
    dev->close();
    delete dev;

    log().setDevice(0);

    m_logInfo = QFileInfo();

    emit logClosed();
}

void Console::systemPrefix(const QString &id)
{
    QColor bgPrefix("saddlebrown");
    QColor fgBracket("goldenrod");
    QColor fgType("darkkhaki");

    colorTell(fgBracket, bgPrefix, "[");
    colorTell(fgType, bgPrefix, QString(" %1 ").arg(id, -4, QLatin1Char(' ')));
    colorTell(fgBracket, bgPrefix, "]");
    colorTell(QColor(), profile()->background(), " ");
}

void Console::systemInfo(const QString &txt)
{
    QColor fgMessage(Qt::white);
    QColor bgMessage(Qt::black);

//    systemPrefix("INFO");
    colorNote(fgMessage, bgMessage, txt);
}

void Console::systemWarn(const QString &txt)
{
    QColor fgMessage("darkorange");
    QColor bgMessage(0x28, 0x28, 0x28);

//    systemPrefix("WARN");
    colorNote(fgMessage, bgMessage, txt);
}

void Console::systemErr(const QString &txt)
{
    QColor fgMessage("silver");
    QColor bgMessage("darkred");

//    systemPrefix("OOPS");
    colorNote(fgMessage, bgMessage, txt);
}

bool Console::processKey(const QKeySequence &key, Group *base)
{
    Group *group = (base == 0)?profile()->rootGroup():base;
    Q_ASSERT(group != 0);
    if (group == 0)
    {
        return false;
    }

    foreach (Accelerator *accelerator, group->accelerators())
    {
        if (accelerator->enabled() && accelerator->key() == key)
        {
            Group *previousGroup = profile()->activeGroup();
            Q_ASSERT(previousGroup != 0);
            profile()->setActiveGroup(accelerator->group());

            if (!accelerator->execute(engine()))
            {
                profile()->setActiveGroup(previousGroup);
                accelerator->setFailed(true);
                continue;
            }

            profile()->setActiveGroup(previousGroup);

            return true;
        }
    }

    foreach (Group *sub, group->groups())
    {
        if (processKey(key, sub))
        {
            return true;
        }
    }

    return false;
}

bool Console::processHyperlink(Hyperlink *link)
{
    QVariant linkTarget(link->link());
    switch (linkTarget.type())
    {
        case QVariant::String:
            commandEntered(linkTarget.toString());
            return true;

        case QVariant::Int:
            return engine()->execute(linkTarget.toInt(), link);

        case QVariant::Url:
            return QDesktopServices::openUrl(linkTarget.toUrl());
    }

    return true;
}

bool Console::processAliases(const QString &cmd)
{
    QElapsedTimer timer;
    timer.start();

    bool ret = false;
    QList<Alias *> aliasList(profile()->rootGroup()->sortedAliases());
    foreach (Alias *alias, aliasList)
    {
        if (!alias->regex().isValid())
        {
            systemErr(tr("Alias %1: %2 (column %3)").arg(alias->name()).arg(alias->regex().errorString()).arg(alias->regex().patternErrorOffset()));
            alias->setFailed(true);
            continue;
        }

        if (alias->enabled() && alias->match(cmd))
        {
            engine()->saveCaptures(alias);

            if (alias->echo())
            {
                document()->command(cmd);

                logLatest();
            }

            Group *previousGroup = profile()->activeGroup();
            Q_ASSERT(previousGroup != 0);
            profile()->setActiveGroup(alias->group());

            if (!alias->execute(engine()))
            {
                profile()->setActiveGroup(previousGroup);
                alias->setFailed(true);

                if (!alias->keepEvaluating())
                {
                    timeProcessing(timer.elapsed() / 1000.0);
                    return true;
                }

                continue;
            }

            profile()->setActiveGroup(previousGroup);

            if (!alias->keepEvaluating())
            {
                timeProcessing(timer.elapsed() / 1000.0);
                return true;
            }

            ret = true;
        }
    }

    timeProcessing(timer.elapsed() / 1000.0);
    return ret;
}

bool Console::processTriggers(const QString &text)
{
//    qDebug() << "processTriggers" << text;

    QList<Trigger *> triggerList(profile()->rootGroup()->sortedTriggers());
    bool omitted = false;
    foreach (Trigger *trigger, triggerList)
    {
        if (!trigger->enabled())
        {
            continue;
        }

        bool matched = false;
        bool keepEvaluating = trigger->keepEvaluating();
        int pos = 0;
//        qDebug() << "  trigger" << trigger->name() << text << pos;
        while (pos < text.length())
        {
            if (!trigger->regex().isValid())
            {
                systemErr(tr("Trigger %1: %2 (column %3)").arg(trigger->name()).arg(trigger->regex().errorString()).arg(trigger->regex().patternErrorOffset()));
                trigger->setFailed(true);
                break;
            }

            matched = trigger->match(text, pos);
            if (matched)
            {
                engine()->saveCaptures(trigger);

                Group *previousGroup = profile()->activeGroup();
                Q_ASSERT(previousGroup != 0);
                profile()->setActiveGroup(trigger->group());

                if (!trigger->execute(engine()))
                {
                    profile()->setActiveGroup(previousGroup);
                    trigger->setFailed(true);
                    keepEvaluating = false;
                    break;
                }

                profile()->setActiveGroup(previousGroup);

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
//            qDebug() << "  pos" << pos;
        }

        if (matched && !keepEvaluating)
        {
            break;
        }
    }

    return !omitted;
}

void Console::backgroundChanged()
{
    ui->output->update();
    ui->scrollback->update();
}

void Console::inputFontChanged()
{
    if (inputFont() != profile()->inputFont())
    {
        setInputFont(profile()->inputFont());
    }
}

void Console::outputFontChanged()
{
    if (outputFont() != profile()->outputFont())
    {
        setOutputFont(profile()->outputFont());
    }
}

void Console::send(const QString &cmd, bool show)
{
    connection()->send(cmd);
    if (show)
    {
        document()->command(cmd);

        logLatest();
    }
}

bool Console::sendGmcp(const QString &msg, const QString &data)
{
    return connection()->sendGmcp(msg, data);
}

void Console::colorTell(const QColor &fg, const QColor &bg, const QString &text)
{
    document()->append(text, fg, bg);
}

void Console::colorNote(const QColor &fg, const QColor &bg, const QString &text)
{
    QString str(text);
    if (!str.endsWith("\n"))
    {
        str.append("\n");
    }
    colorTell(fg, bg, str);
    logLatest();
}

int Console::hyperlink(const QString &str, const QColor &fg, const QColor &bg, Hyperlink *info)
{
    Q_ASSERT(info != 0);

    document()->appendLink(str, info->link().toString(), fg, bg, info->hint());

//    ui->output->hyperlink(fg, bg, str, info->link().toUrl(), info->hint());

//    int id = buffer()->link(str, fg, bg, info);

    return 0;
//    return id;
}

QString Console::playMedia(QMediaContent content)
{
    m_media->setMedia(content);
    m_media->play();

    if (m_media->error() != QMediaPlayer::NoError)
    {
        return m_media->errorString();
    }

    return QString();
}

void Console::timeLatency(double time)
{
    m_latency.append(time);
    if (m_latency.size() > m_maxPackets)
    {
        m_latency.pop_front();
    }
}

void Console::timeProcessing(double time)
{
    m_processing.append(time);
    if (m_processing.size() > m_maxPackets)
    {
        m_processing.pop_front();
    }
}

double Console::meanLatency() const
{
    if (m_latency.isEmpty())
    {
        return 0.0;
    }

    double total = 0.0;
    foreach (double value, m_latency)
    {
        total += value;
    }

    return total / m_latency.size();
}

double Console::meanProcessing() const
{
    if (m_processing.isEmpty())
    {
        return 0.0;
    }

    double total = 0.0;
    foreach (double value, m_processing)
    {
        total += value;
    }

    return total / m_processing.size();
}

void Console::closeEvent(QCloseEvent *e)
{
    if (profile()->isDirty())
    {
        QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Modified Profile"), tr("Profile '%1' has been modified.\nDo you wish to save changes before closing?").arg(profile()->name()), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
        switch (answer)
        {
            case QMessageBox::Cancel:
                e->ignore();
                return;

            case QMessageBox::No:
                break;

            case QMessageBox::Yes:
                QString filename(profile()->filename());
                if (filename.isEmpty())
                {
                    filename = QFileDialog::getSaveFileName(this, tr("Save Profile"), QString(), tr("Mudder Profiles (*.mp);;All files (*)"));
                }

                if (!filename.isEmpty())
                {
                    if (!saveProfile(filename))
                    {
                        QMessageBox::critical(this, tr("Profile Error"), tr("Unable to save profile '%1' to disk.\nCheck to make sure the file is not in use by another application and there is room on the storage device.").arg(profile()->name()));
                        e->ignore();
                        return;
                    }
                }
                break;
        }
    }

    e->accept();
}

void Console::mouseMoveEvent(QMouseEvent *e)
{
    qDebug() << "mouseMove" << e->pos();

    if (!document())
    {
        return;
    }

    if (m_mousePressed)
    {
        int pos = document()->documentLayout()->hitTest(e->pos(), Qt::FuzzyHit);
        if (pos < 0)
        {
            e->ignore();
            return;
        }

        m_selectionEnd = pos;

        document()->select(m_selectionStart, m_selectionEnd);

        if (e->y() < 10)
        {
            scrollUp(1);
        }
        else if (e->y() > height() - 10)
        {
            scrollDown(1);
        }
    }
    else
    {
        QString anchor(document()->documentLayout()->anchorAt(e->pos()));
        qDebug() << "anchor" << anchor;
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

void Console::mousePressEvent(QMouseEvent *e)
{
    if (!document())
    {
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        m_clickPos = document()->documentLayout()->hitTest(e->pos(), Qt::ExactHit);

        int pos = document()->documentLayout()->hitTest(e->pos(), Qt::FuzzyHit);
        if (pos < 0)
        {
            return;
        }

        m_mousePressed = true;
        m_selectionStart = pos;
        m_selectionEnd = pos;

        qDebug() << "mousePress" << m_selectionStart << m_selectionEnd;
    }
}

void Console::mouseReleaseEvent(QMouseEvent *e)
{
//    QPoint pt(textCursor(e->pos()));
//    handleMouseEvent(e, pt);

    if (document() && e->button() == Qt::RightButton)
    {
        QMenu *popup = new QMenu(this);

        if (document()->hasSelection())
        {
            QAction *actionCopy = new QAction(tr("&Copy"), this);
            actionCopy->setStatusTip(tr("Copy selected text to clipboard"));
            connect(actionCopy, SIGNAL(triggered()), this, SLOT(copy()));

            QAction *actionCopyHtml = new QAction(tr("Copy as &HTML"), this);
            actionCopyHtml->setStatusTip(tr("Copy selected text to clipboard as HTML"));
            connect(actionCopyHtml, SIGNAL(triggered()), this, SLOT(copyHtml()));

            QAction *actionSelectNone = new QAction(tr("Select &None"), this);
            actionSelectNone->setStatusTip(tr("Removes the current text selection"));
            connect(actionSelectNone, SIGNAL(triggered()), document(), SLOT(selectNone()));

            popup->addAction(actionCopy);
            popup->addAction(actionCopyHtml);
            popup->addSeparator();
            popup->addAction(actionSelectNone);
        }

        QAction *actionSelectAll = new QAction(tr("Select &All"), this);
        actionSelectAll->setStatusTip(tr("Select all buffered output text"));
        connect(actionSelectAll, SIGNAL(triggered()), document(), SLOT(selectAll()));

        QAction *actionClearBuffer = new QAction(tr("C&lear output buffer"), this);
        actionClearBuffer->setStatusTip(tr("Deletes all text stored in the output text buffer"));
        connect(actionClearBuffer, SIGNAL(triggered()), document(), SLOT(clear()));

        popup->addAction(actionSelectAll);
        popup->addSeparator();
        popup->addAction(actionClearBuffer);

        popup->popup(mapToGlobal(e->pos()), popup->actions().at(0));
    }
    else if (e->button() == Qt::MiddleButton)
    {
        scrollTo(document()->blockCount());
    }
    else if (e->button() == Qt::LeftButton)
    {
        qDebug() << "mouseRelease" << m_selectionStart << m_selectionEnd;

        // TODO: don't clear selection when clicking within the selected text?
        if (document()->documentLayout()->hitTest(e->pos(), Qt::ExactHit) == m_clickPos)
        {
            document()->selectNone();

            m_selectionStart = 0;
            m_selectionEnd = 0;
        }

        m_mousePressed = false;
        m_clickPos = -1;
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
