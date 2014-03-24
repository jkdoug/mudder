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
#include <QtDebug>


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
    qRegisterMetaType<Logger::MessageType>("Logger::MessageType");
    qRegisterMetaType<Logger::MessageContextFlags>("Logger::MessageContextFlags");
}

Logger::~Logger()
{
    clear();
}

void Logger::initialize()
{
    if (m_initialized)
    {
        return;
    }

    m_formattingEngines << &DefaultFormattingEngine::instance();
    m_formattingEngines << &RichTextFormattingEngine::instance();
    m_formattingEngines << &XmlFormattingEngine::instance();
    m_formattingEngines << &HtmlFormattingEngine::instance();
    m_formattingEngines << &QtMsgFormattingEngine::instance();
    m_defaultFormattingEngine = "Default";

    m_loggerEngineFactory.registerFactoryInterface("def.FactoryTag.File", &FileLoggerEngine::factory);

    LoggerEngine *engine = QtMsgLoggerEngine::instance();
    engine->installFormattingEngine(&QtMsgFormattingEngine::instance());
    attachLoggerEngine(engine, true);
    toggleQtMsgEngine(false);

    engine = ConsoleLoggerEngine::instance();
    engine->installFormattingEngine(&DefaultFormattingEngine::instance());
    attachLoggerEngine(engine, true);
    toggleConsoleEngine(false);

    m_initialized = true;
}

void Logger::finalize()
{
    clear();
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
            LOG_ERROR(tr("New logger engine could not be added, failed during initialization."));
            delete engine;
            engine = 0;
            return false;
        }
    }

    if (engine)
    {
        engine->setObjectName(engine->name());
        m_loggerEngines << engine;
        connect(this, SIGNAL(newMessage(QString, Logger::MessageType, Logger::MessageContext, QList<QVariant>)),
                engine, SLOT(newMessages(QString, Logger::MessageType, Logger::MessageContext, QList<QVariant>)));
    }

    emit loggerEngineCountChanged(engine, EngineAdded);
    return true;
}

bool Logger::detachLoggerEngine(LoggerEngine *engine, bool del)
{
    if (engine && m_loggerEngines.removeOne(engine))
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
    if (m_globalLogLevel == logLevel)
    {
        return;
    }

    m_globalLogLevel = logLevel;

    LOG_INFO(tr("Global log level changed to %1.").arg(logLevelToString(logLevel)));
}

Logger::MessageType Logger::globalLogLevel() const
{
    return m_globalLogLevel;
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
    strings << "Engine";
    return strings;
}

void Logger::deleteAllLoggerEngines()
{
    foreach (LoggerEngine *engine, m_loggerEngines)
    {
        if (engine)
        {
            delete engine;
        }
    }
    m_loggerEngines.clear();
}

void Logger::disableAllLoggerEngines()
{
    foreach (LoggerEngine *engine, m_loggerEngines)
    {
        if (engine)
        {
            engine->setActive(false);
        }
    }
}

void Logger::enableAllLoggerEngines()
{
    foreach (LoggerEngine *engine, m_loggerEngines)
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
    foreach (FormattingEngine *engine, m_formattingEngines)
    {
        names << engine->name();
    }
    return names;
}

FormattingEngine * Logger::formattingEngineReference(const QString &name)
{
    foreach (FormattingEngine *engine, m_formattingEngines)
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
        m_formattingEngines << engine;
    }
}

FormattingEngine * Logger::formattingEngineReferenceFromExtension(const QString &extension)
{
    foreach (FormattingEngine *engine, m_formattingEngines)
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
    if (index < 0 || index >= m_formattingEngines.count())
    {
        return 0;
    }

    return m_formattingEngines.at(index);
}

QString Logger::defaultFormattingEngine() const
{
    return m_defaultFormattingEngine;
}

void Logger::registerLoggerEngineFactory(const QString &tag, LoggerFactoryInterface<LoggerEngine> *factory)
{
    m_loggerEngineFactory.registerFactoryInterface(tag, factory);
}

QStringList Logger::availableLoggerEnginesInFactory() const
{
    return m_loggerEngineFactory.tags();
}

inline int Logger::attachedFormattingEngineCount() const
{
    return m_formattingEngines.count();
}

QStringList Logger::attachedLoggerEngineNames() const
{
    QStringList names;
    foreach (LoggerEngine *engine, m_loggerEngines)
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
    foreach (LoggerEngine *engine, m_loggerEngines)
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
    foreach (LoggerEngine *engine, m_loggerEngines)
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
    foreach (LoggerEngine *engine, m_loggerEngines)
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
                        isMatch = (QDir::toNativeSeparators(cleanPath1).compare(QDir::toNativeSeparators(cleanPath2), Qt::CaseInsensitive) == 0);
#else
                        isMatch = (QDir::toNativeSeparators(cleanPath1).compare(QDir::toNativeSeparators(cleanPath2), Qt::CaseSensitive) == 0);
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
    foreach (LoggerEngine *engine, m_loggerEngines)
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
    QString engineName(QString("%1_%2").arg(tag).arg(count));
    while (attachedLoggerEngineNames().contains(engineName))
    {
        QString countString(QString("%1").arg(count));
        engineName.chop(countString.length());
        engineName.append(QString("%1").arg(++count));
    }

    LoggerEngine * engine = m_loggerEngineFactory.createInstance(tag);
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
        qDebug() << "Attempting to attach logger engines with duplicate names; this is not allowed. Name: " << engineName;
        return 0;
    }

    QPointer<FileLoggerEngine> fileEngine;
    QPointer<LoggerEngine> engine = m_loggerEngineFactory.createInstance("def.FactoryTag.File");
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
    if (m_loggerEngines.contains(QtMsgLoggerEngine::instance()))
    {
        QtMsgLoggerEngine::instance()->setActive(toggle);
    }
}

bool Logger::qtMsgEngineActive() const
{
    if (m_loggerEngines.contains(QtMsgLoggerEngine::instance()))
    {
        return QtMsgLoggerEngine::instance()->isActive();
    }
    return false;
}

void Logger::toggleConsoleEngine(bool toggle)
{
    if (m_loggerEngines.contains(ConsoleLoggerEngine::instance()))
    {
        ConsoleLoggerEngine::instance()->setActive(toggle);
    }
}

bool Logger::consoleEngineActive() const
{
    if (m_loggerEngines.contains(ConsoleLoggerEngine::instance()))
    {
        return ConsoleLoggerEngine::instance()->isActive();
    }
    return false;
}

void Logger::clear()
{
    for (int n = 0; n < m_loggerEngines.count(); n++)
    {
        if (m_loggerEngines.at(n))
        {
            if (m_loggerEngines.at(n) != QtMsgLoggerEngine::instance() &&
                m_loggerEngines.at(n) != ConsoleLoggerEngine::instance())
            {
                delete m_loggerEngines.at(n);
            }
        }

    }
    m_loggerEngines.clear();
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

    if (type > m_globalLogLevel)
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
