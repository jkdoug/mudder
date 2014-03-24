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


#include "logger.h"
#include "loggerengine.h"
#include "loggerexportable.h"
#include "loggerfactory.h"
#include "consoleloggerengine.h"
#include "fileloggerengine.h"
#include "qtmsgloggerengine.h"
#include "formattingengine.h"
#include "defaultformattingengine.h"
#include "htmlformattingengine.h"
#include "qtmsgformattingengine.h"
#include "richtextformattingengine.h"
#include "xmlformattingengine.h"
#include <QCoreApplication>
#include <QDir>
#include <QMutex>
#include <QPointer>
#include <QtDebug>

const quint32 MARKER_LOGGER_CONFIG_TAG = 0xFACE10FF;


struct LoggerPrivate
{
    LoggerPrivate()
    {
        defaultFormattingEngine = "Uninitialized";
        globalLogLevel = Logger::Debug;
        initialized = false;
        rememberSessionConfig = false;
        priorityFormattingEngine = 0;
        sessionPath = QCoreApplication::applicationDirPath() + "Session";
        settingsEnabled = true;
    }

    LoggerFactory<LoggerEngine> loggerEngineFactory;
    QList<QPointer<LoggerEngine> > loggerEngines;
    QList<QPointer<FormattingEngine> > formattingEngines;
    QString defaultFormattingEngine;
    Logger::MessageType globalLogLevel;
    bool initialized;
    bool isQtMessageHandler;
    bool rememberSessionConfig;
    QPointer<FormattingEngine> priorityFormattingEngine;
    QString sessionPath;
    bool settingsEnabled;
};


Logger * Logger::m_instance = 0;

Logger * Logger::instance()
{
    static QMutex mutex;
    if (!m_instance)
    {
        mutex.lock();
        if (!m_instance)
        {
            m_instance = new Logger;
        }
        mutex.unlock();
    }

    return m_instance;
}

Logger::Logger()
{
    m_d = new LoggerPrivate;

    qRegisterMetaType<Logger::MessageType>("Logger::MessageType");
    qRegisterMetaType<Logger::MessageContextFlags>("Logger::MessageContextFlags");
}

Logger::~Logger()
{
    clear();
    delete m_d;
}

void Logger::initialize(const QString &configFileName)
{
    if (m_d->initialized)
    {
        return;
    }

    m_d->formattingEngines << &DefaultFormattingEngine::instance();
    m_d->formattingEngines << &RichTextFormattingEngine::instance();
    m_d->formattingEngines << &XmlFormattingEngine::instance();
    m_d->formattingEngines << &HtmlFormattingEngine::instance();
    m_d->formattingEngines << &QtMsgFormattingEngine::instance();
    m_d->defaultFormattingEngine = "Default";

    m_d->loggerEngineFactory.registerFactoryInterface("def.FactoryTag.File", &FileLoggerEngine::factory);

    LoggerEngine *engine = QtMsgLoggerEngine::instance();
    engine->installFormattingEngine(&QtMsgFormattingEngine::instance());
    attachLoggerEngine(engine, true);
    toggleQtMsgEngine(false);

    engine = ConsoleLoggerEngine::instance();
    engine->installFormattingEngine(&DefaultFormattingEngine::instance());
    attachLoggerEngine(engine, true);
    toggleConsoleEngine(false);

//    readSettings();       // TODO

    if (m_d->rememberSessionConfig)
    {
        loadSessionConfig(configFileName);
    }

    m_d->initialized = true;
}

void Logger::finalize(const QString &configFileName)
{
    if (m_d->rememberSessionConfig)
    {
        saveSessionConfig(configFileName);
    }

    clear();
}

bool Logger::loadSessionConfig(QString configFileName)
{
    if (configFileName.isEmpty())
    {
        configFileName = m_d->sessionPath + QDir::separator() + "last_stored.logconfig";
    }

//    LOG_DEBUG(tr("Logging configuration import started from ") + file_name);
    QFile file(configFileName);
    if (!file.open(QIODevice::ReadOnly))
    {
//        LOG_INFO(tr("Logging configuration import failed from ") + file_name + tr(". This file could not be opened in read mode."));
        return false;
    }

    QDataStream stream(&file);

    quint32 ui32;
    stream >> ui32;
    if (ui32 != MARKER_LOGGER_CONFIG_TAG)
    {
        file.close();
//        LOG_INFO(tr("Logging configuration import failed from ") + file_name + tr(". The file contains invalid data or does not exist."));
        return false;
    }

    quint32 globalLogLevel;
    stream >> globalLogLevel;

    quint32 importCount;
    stream >> importCount;

    bool success = true;
    QList<LoggerEngine *> engines;
    for (quint32 i = 0; i < importCount; i++)
    {
        if (!success)
        {
            break;
        }

        QString tag;
        stream >> tag;
//        LOG_DEBUG(tr("Creating logger factory instance with tag: ") + tag);
        LoggerEngine *engine = m_d->loggerEngineFactory.createInstance(tag);
        if (engine)
        {
            QString name;
            stream >> name;
            engine->setName(name);
            LoggerExportable *logExport = qobject_cast<LoggerExportable *>(engine);
            if (logExport)
            {
                logExport->importBinary(stream);
                engines << engine;
            }
            else
            {
//                LOG_WARNING(tr("Logger engine could not be constructed for factory tag: ") + tag);
                success = false;
            }
        }
        else
        {
            success = false;
        }
    }

    if (success)
    {
        QList<LoggerEngine*> loggers;
        foreach (LoggerEngine *engine, m_d->loggerEngines)
        {
            LoggerExportable *logExport = qobject_cast<LoggerExportable *>(engine);
            if (logExport)
            {
                loggers << engine;
            }
        }

        foreach (LoggerEngine *engine, loggers)
        {
            detachLoggerEngine(engine);
        }

        foreach (LoggerEngine *engine, engines)
        {
            if (!attachLoggerEngine(engine))
            {
                success = false;
            }
        }
    }
    else
    {
        foreach (LoggerEngine *engine, engines)
        {
            delete engine;
        }
    }

    bool complete = true;
    if (success)
    {
        stream >> importCount;

        QString currentName;
        QString currentEngine;
        bool isActive;
        MessageContextFlags messageContextFlags;
        for (quint32 i = 0; i < importCount; i++)
        {
            if (!success)
            {
                break;
            }

            stream >> currentName;
            stream >> currentEngine;
            stream >> isActive;

            quint32 context;
            stream >> context;
            messageContextFlags = (MessageContextFlags)context;

            LoggerEngine *engine = loggerEngineReference(currentName);
            if (engine)
            {
//                LOG_DEBUG(tr("Restoring configuration for logger engine: ") + currentName);
                engine->installFormattingEngine(formattingEngineReference(currentEngine));
                engine->setActive(isActive);
                engine->setMessageContexts(messageContextFlags);
            }
            else
            {
//                LOG_DEBUG(tr("Found logger engine configuration for an engine which does not exist yet with name: ") + currentName);
                complete = false;
            }
        }
    }

    file.close();
    if (success)
    {
        setGlobalLogLevel((Logger::MessageType)globalLogLevel);
        if (complete)
        {
//            LOG_INFO(tr("Successfully imported logging configuration (complete) imported from ") + file_name);
        }
        else
        {
//            LOG_WARNING(tr("Logging configuration successfully (incomplete) imported from ") + file_name);
        }

        return true;
    }

//    LOG_INFO(tr("Logging configuration import failed from ") + file_name);
    return false;
}

bool Logger::saveSessionConfig(QString configFileName) const
{
    if (!m_d->settingsEnabled)
    {
        return false;
    }

    if (configFileName.isEmpty())
        configFileName = m_d->sessionPath + QDir::separator() + "last_stored.logconfig";

//    LOG_DEBUG(tr("Logging configuration export started to ") + file_name);

    QFileInfo fileInfo(configFileName);
    if (!fileInfo.dir().exists())
    {
        fileInfo.dir().mkpath(fileInfo.path());
    }

    QFile file(configFileName);
    if (!file.open(QIODevice::WriteOnly))
    {
//        LOG_DEBUG(tr("Logging configuration export failed to ") + file_name + tr(". The file could not be opened in WriteOnly mode."));
        return false;
    }

    QDataStream stream(&file);
    stream << MARKER_LOGGER_CONFIG_TAG;

    QList<LoggerExportable *> exportLoggers;
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine)
        {
            LoggerExportable *logExport = qobject_cast<LoggerExportable *>(engine);
            if (logExport)
            {
                exportLoggers << logExport;
            }
        }
    }

    stream << (quint32)m_d->globalLogLevel;
    stream << (quint32)exportLoggers.count();

    bool success = true;
    foreach (LoggerExportable *engine, exportLoggers)
    {
//        LOG_DEBUG(tr("Exporting logger factory instance: ") + engine->factoryTag());
        stream << engine->factoryTag();
        stream << engine->instanceName();
        success = engine->exportBinary(stream);

        if (!success)
        {
            break;
        }
    }

    if (success)
    {
        stream << (quint32)m_d->loggerEngines.count();
        foreach (LoggerEngine *engine, m_d->loggerEngines)
        {
            if (engine)
            {
//                LOG_DEBUG(tr("Saving configuration for logger engine: ") + engine->name());
                stream << engine->name();
                stream << engine->formattingEngineName();
                stream << engine->isActive();
                stream << (quint32)engine->messageContexts();
            }
        }
    }

    file.close();
    if (success)
    {
//        LOG_INFO(tr("Successfully exported logging configuration exported to ") + file_name);
        return true;
    }

//    LOG_ERROR(tr("Logging configuration export failed to ") + file_name);
    return false;
}

bool Logger::attachLoggerEngine(LoggerEngine *engine, bool init)
{
    if (!engine)
    {
        return false;
    }

    if (attachedLoggerEngineNames().contains(engine->name()))
    {
        qDebug() << "Attempting to attach logger engines with duplicate names; this is not allowed. Name:" << engine->name();
        return false;
    }

    if (init)
    {
        bool initResult = engine->initialize();
        if (!initResult)
        {
//            LOG_ERROR(tr("New logger engine could not be added, failed during initialization."));
            delete engine;
            engine = 0;
            return false;
        }
    }

    if (engine)
    {
        engine->setObjectName(engine->name());
        m_d->loggerEngines << engine;
        connect(this, SIGNAL(newMessage(QString, Logger::MessageType, Logger::MessageContext, QList<QVariant>)),
                engine, SLOT(newMessages(QString, Logger::MessageType, Logger::MessageContext, QList<QVariant>)));
    }

    emit loggerEngineCountChanged(engine, EngineAdded);
    return true;
}

bool Logger::detachLoggerEngine(LoggerEngine *engine, bool del)
{
    if (engine && m_d->loggerEngines.removeOne(engine))
    {
        emit loggerEngineCountChanged(engine, EngineRemoved);
        if (del)
        {
            delete engine;
        }
        return true;
    }

    return false;
}

void Logger::setGlobalLogLevel(Logger::MessageType logLevel)
{
    if (m_d->globalLogLevel == logLevel)
    {
        return;
    }

    m_d->globalLogLevel = logLevel;

//    writeSettings();  // TODO
//    LOG_INFO("Global log level changed to " + logLevelToString(logLevel));
}

Logger::MessageType Logger::globalLogLevel() const
{
    return m_d->globalLogLevel;
}
QString Logger::logLevelToString(Logger::MessageType logLevel) const
{
    switch (logLevel)
    {
    case None:
        return "None";

    case Info:
        return "Info";

    case Warning:
        return "Warning";

    case Error:
        return "Error";

    case Fatal:
        return "Fatal";

    case Debug:
        return "Debug";

    case Trace:
        return "Trace";

    case AllLogLevels:
        return "All Log Levels";
    }

    return QString();
}

Logger::MessageType Logger::stringToLogLevel(const QString &logLevel) const
{
    if (logLevel.compare("Info") == 0)
    {
        return Info;
    }
    else if (logLevel.compare("Warning") == 0)
    {
        return Warning;
    }
    else if (logLevel.compare("Error") == 0)
    {
        return Error;
    }
    else if (logLevel.compare("Fatal") == 0)
    {
        return Fatal;
    }
    else if (logLevel.compare("Debug") == 0)
    {
        return Debug;
    }
    else if (logLevel.compare("Trace") == 0)
    {
        return Trace;
    }
    else if (logLevel.compare("All Log Levels") == 0)
    {
        return AllLogLevels;
    }
    return None;
}

QStringList Logger::allLogLevelStrings() const
{
    QStringList strings;
    strings << "None";
    strings << "Information";
    strings << "Warning";
    strings << "Error";
    strings << "Fatal";
#ifndef QT_NO_DEBUG
    strings << "Debug";
    strings << "Trace";
#endif
    strings << "All Log Levels";
    return strings;
}

QString Logger::messageContextsToString(Logger::MessageContextFlags contexts) const
{
    QStringList contextList;
    if (contexts.testFlag(SystemWideMessages))
    {
        contextList << "System";
    }
    if (contexts.testFlag(PriorityMessages))
    {
        contextList << "Priority";
    }
    if (contexts.testFlag(EngineSpecificMessages))
    {
        contextList << "Engine";
    }

    if (contextList.isEmpty())
    {
        contextList << "None";
    }

    return contextList.join(",");
}

Logger::MessageContextFlags Logger::stringToMessageContexts(const QString &contexts) const
{
    MessageContextFlags flags = 0;
    if (contexts.contains("System"))
    {
        flags |= SystemWideMessages;
    }
    if (contexts.contains("Priority"))
    {
        flags |= PriorityMessages;
    }
    if (contexts.contains("Engine"))
    {
        flags |= EngineSpecificMessages;
    }

    return flags;
}

QStringList Logger::allMessageContextStrings() const
{
    QStringList strings;
    strings << "None";
    strings << "System";
    strings << "Priority";
    strings << "Engine";
    return strings;
}

void Logger::deleteAllLoggerEngines()
{
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine)
        {
            delete engine;
        }
    }
    m_d->loggerEngines.clear();
}

void Logger::disableAllLoggerEngines()
{
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine)
        {
            engine->setActive(false);
        }
    }
}

void Logger::enableAllLoggerEngines()
{
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine)
        {
            engine->setActive(true);
        }
    }
}

void Logger::deleteEngine(const QString &engineName)
{
    LoggerEngine *engine = loggerEngineReference(engineName);
    if (!engine)
    {
        return;
    }

    delete engine;
}

void Logger::enableEngine(const QString &engineName)
{
    LoggerEngine *engine = loggerEngineReference(engineName);
    if (!engine)
    {
        return;
    }

    engine->setActive(true);
}

void Logger::disableEngine(const QString &engineName)
{
    LoggerEngine *engine = loggerEngineReference(engineName);
    if (!engine)
    {
        return;
    }

    engine->setActive(false);
}

QStringList Logger::availableFormattingEnginesInFactory() const
{
    QStringList names;
    foreach (FormattingEngine *engine, m_d->formattingEngines)
    {
        names << engine->name();
    }
    return names;
}

FormattingEngine * Logger::formattingEngineReference(const QString &name)
{
    foreach (FormattingEngine *engine, m_d->formattingEngines)
    {
        if (engine->name().compare(name) == 0)
        {
            return engine;
        }
    }
    return 0;
}

void Logger::registerFormattingEngine(FormattingEngine *engine)
{
    if (engine)
    {
        m_d->formattingEngines << engine;
    }
}

FormattingEngine * Logger::formattingEngineReferenceFromExtension(const QString &extension)
{
    foreach (FormattingEngine *engine, m_d->formattingEngines)
    {
        if (engine->fileExtension().compare(extension) == 0)
        {
            return engine;
        }
    }
    return 0;
}

FormattingEngine * Logger::formattingEngineReferenceAt(int index)
{
    if (index < 0 || index >= m_d->formattingEngines.count())
    {
        return 0;
    }

    return m_d->formattingEngines.at(index);
}

QString Logger::defaultFormattingEngine() const
{
    return m_d->defaultFormattingEngine;
}

void Logger::registerLoggerEngineFactory(const QString &tag, LoggerFactoryInterface<LoggerEngine> *factory)
{
    m_d->loggerEngineFactory.registerFactoryInterface(tag, factory);
}

QStringList Logger::availableLoggerEnginesInFactory() const
{
    return m_d->loggerEngineFactory.tags();
}

inline int Logger::attachedFormattingEngineCount() const
{
    return m_d->formattingEngines.count();
}

QStringList Logger::attachedLoggerEngineNames() const
{
    QStringList names;
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine)
        {
            names << engine->name();
        }
    }
    return names;
}

int Logger::attachedLoggerEngineCount() const
{
    int count = 0;
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine)
        {
            count++;
        }
    }
    return count;
}

LoggerEngine * Logger::loggerEngineReference(const QString &engineName)
{
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine && engine->name().compare(engineName) == 0)
        {
            return engine;
        }
    }
    return 0;
}

LoggerEngine * Logger::loggerEngineReferenceForFile(const QString &filePath)
{
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine)
        {
            FileLoggerEngine * fileEngine = qobject_cast<FileLoggerEngine *>(engine);
            if (fileEngine)
            {
                bool isMatch = false;
                QFileInfo fileInfo1(fileEngine->fileName());
                QFileInfo fileInfo2(filePath);
                if (fileInfo1.exists() && fileInfo2.exists())
                {
                    isMatch = (fileInfo1 == fileInfo2);
                }
                else
                {
                    QString cleanPath1(QDir::cleanPath(fileEngine->fileName()));
                    QString cleanPath2(QDir::cleanPath(filePath));
                    if (cleanPath1.size() == cleanPath2.size())
                    {
#ifdef Q_OS_WIN
                        isMatch = (QDir::toNativeSeparators(cleanPath1).compare(QDir::toNativeSeparators(cleanPath2),Qt::CaseInsensitive) == 0);
#else
                        isMatch = (QDir::toNativeSeparators(cleanPath1).compare(QDir::toNativeSeparators(cleanPath2),Qt::CaseSensitive) == 0);
#endif
                    }
                    else
                    {
                        isMatch = false;
                    }
                }

                if (isMatch)
                {
                    return fileEngine;
                }
            }
        }
    }

    return 0;
}

LoggerEngine * Logger::loggerEngineReferenceAt(int index)
{
    int validCount = 0;
    foreach (LoggerEngine *engine, m_d->loggerEngines)
    {
        if (engine)
        {
            if (validCount == index)
            {
                return engine;
            }
            validCount++;
        }
    }
    return 0;
}

LoggerEngine * Logger::newLoggerEngine(QString tag, FormattingEngine * formattingEngine)
{
    int count = 0;
    QString engineName = QString("%1_%2").arg(tag).arg(count);
    while (attachedLoggerEngineNames().contains(engineName))
    {
        QString countString = QString("%1").arg(count);
        engineName.chop(countString.length());
        engineName.append(QString("%1").arg(++count));
    }

    LoggerEngine * engine = m_d->loggerEngineFactory.createInstance(tag);
    if (!engine)
    {
        return 0;
    }
    engine->setName(engineName);

    if (formattingEngine)
    {
        engine->installFormattingEngine(formattingEngine);
    }

    return engine;
}
LoggerEngine* Logger::newFileEngine(const QString &engineName, const QString &fileName, const QString &formattingEngine)
{
    if (fileName.isEmpty())
    {
        return 0;
    }

    if (attachedLoggerEngineNames().contains(engineName))
    {
        qDebug() << "Attempting to attach logger engines with duplicate names; this is not allowed. Name:" << engineName;
        return 0;
    }

    QPointer<FileLoggerEngine> fileEngine;
    QPointer<LoggerEngine> engine = m_d->loggerEngineFactory.createInstance("def.FactoryTag.File");
    Q_ASSERT(engine);
    engine->setName(engineName);

    fileEngine = qobject_cast<FileLoggerEngine *>(engine);
    fileEngine->setFileName(fileName);

    if (formattingEngineReference(formattingEngine))
    {
        FormattingEngine* formattingEngineRef = formattingEngineReference(formattingEngine);
        if (!formattingEngineRef)
        {
            delete engine;
            return 0;
        }
        engine->installFormattingEngine(formattingEngineRef);
    }
    else
    {
        QFileInfo fileInfo(fileName);
        FormattingEngine * formattingEngineRef = formattingEngineReferenceFromExtension(fileInfo.suffix());
        if (!formattingEngineRef)
        {
            delete engine;
            return 0;
        }
        engine->installFormattingEngine(formattingEngineRef);
    }

    if (attachLoggerEngine(engine, true))
    {
        return engine;
    }

    delete engine;
    return 0;
}

void Logger::toggleQtMsgEngine(bool toggle)
{
    if (m_d->loggerEngines.contains(QtMsgLoggerEngine::instance()))
    {
        QtMsgLoggerEngine::instance()->setActive(toggle);
    }
}

bool Logger::qtMsgEngineActive() const
{
    if (m_d->loggerEngines.contains(QtMsgLoggerEngine::instance()))
    {
        return QtMsgLoggerEngine::instance()->isActive();
    }
    return false;
}

void Logger::toggleConsoleEngine(bool toggle)
{
    if (m_d->loggerEngines.contains(ConsoleLoggerEngine::instance()))
    {
        ConsoleLoggerEngine::instance()->setActive(toggle);
    }
}

bool Logger::consoleEngineActive() const
{
    if (m_d->loggerEngines.contains(ConsoleLoggerEngine::instance()))
    {
        return ConsoleLoggerEngine::instance()->isActive();
    }
    return false;
}

void Logger::clear()
{
    for (int i = 0; i < m_d->loggerEngines.count(); i++)
    {
        if (m_d->loggerEngines.at(i))
        {
            if (m_d->loggerEngines.at(i) != QtMsgLoggerEngine::instance() &&
                m_d->loggerEngines.at(i) != ConsoleLoggerEngine::instance())
            {
                delete m_d->loggerEngines.at(i);
            }
        }

    }
    m_d->loggerEngines.clear();
}

void Logger::logMessage(const QString &engineName, MessageType type, const QVariant &message,
                        const QVariant &msg1, const QVariant &msg2, const QVariant &msg3,
                        const QVariant &msg4, const QVariant &msg5, const QVariant &msg6,
                        const QVariant &msg7, const QVariant &msg8, const QVariant &msg9)
{
#ifdef QT_NO_DEBUG
    if (type == Debug || type == Trace)
    {
        return;
    }
#endif

    if (type == AllLogLevels || type == None)
    {
        return;
    }

    if (type > m_d->globalLogLevel)
    {
        return;
    }

    QList<QVariant> contents;
    contents << message;
    if (!msg1.isNull())
    {
        contents << msg1;
    }
    if (!msg2.isNull())
    {
        contents << msg2;
    }
    if (!msg3.isNull())
    {
        contents << msg3;
    }
    if (!msg4.isNull())
    {
        contents << msg4;
    }
    if (!msg5.isNull())
    {
        contents << msg5;
    }
    if (!msg6.isNull())
    {
        contents << msg6;
    }
    if (!msg7.isNull())
    {
        contents << msg7;
    }
    if (!msg8.isNull())
    {
        contents << msg8;
    }
    if (!msg9.isNull())
    {
        contents << msg9;
    }

    emit newMessage(engineName, type, engineName.isEmpty()?SystemWideMessages:EngineSpecificMessages, contents);
}
