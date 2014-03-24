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


#ifndef LOGGER_H
#define LOGGER_H

#include "logging_global.h"
#include "loggerfactory.h"
#include <QObject>
#include <QPointer>
#include <QVariant>

class LoggerEngine;
class FormattingEngine;

class LOGGINGSHARED_EXPORT Logger : public QObject
{
    Q_OBJECT

public:
    enum MessageType
    {
        None            = 1 << 0,
        Info            = 1 << 1,
        Warning         = 1 << 2,
        Error           = 1 << 3,
        Fatal           = 1 << 4,
        Debug           = 1 << 5,
        Trace           = 1 << 6,
        AllLogLevels    = Info | Warning | Error | Fatal | Debug | Trace
    };
    Q_DECLARE_FLAGS(MessageTypeFlags, MessageType)
    Q_FLAGS(MessageTypeFlags)
    Q_ENUMS(MessageType)

    enum MessageContext
    {
        NoMessageContext        = 1 << 0,
        SystemWideMessages      = 1 << 1,
        EngineSpecificMessages  = 1 << 2,
        AllMessageContexts      = SystemWideMessages | EngineSpecificMessages
    };
    Q_DECLARE_FLAGS(MessageContextFlags, MessageContext)
    Q_FLAGS(MessageContextFlags)
    Q_ENUMS(MessageContext)

    enum EngineChangeIndication
    {
        EngineAdded,
        EngineRemoved
    };
    Q_ENUMS(EngineChangeIndication)

    static Logger * instance();
    ~Logger();

    void initialize();
    void finalize();

    bool attachLoggerEngine(LoggerEngine *engine, bool init = true);
    bool detachLoggerEngine(LoggerEngine *engine, bool del = true);

    void setGlobalLogLevel(Logger::MessageType logLevel);
    Logger::MessageType globalLogLevel() const;
    QString logLevelToString(Logger::MessageType logLevel) const;
    Logger::MessageType stringToLogLevel(const QString &logLevel) const;
    QStringList allLogLevelStrings() const;
    QString messageContextsToString(Logger::MessageContextFlags contexts) const;
    Logger::MessageContextFlags stringToMessageContexts(const QString &contexts) const;
    QStringList allMessageContextStrings() const;

    void deleteAllLoggerEngines();
    void disableAllLoggerEngines();
    void enableAllLoggerEngines();
    void deleteEngine(const QString &engineName);
    void enableEngine(const QString &engineName);
    void disableEngine(const QString &engineName);

    QStringList availableFormattingEnginesInFactory() const;
    FormattingEngine * formattingEngineReference(const QString &name);
    void registerFormattingEngine(FormattingEngine * engine);
    FormattingEngine * formattingEngineReferenceFromExtension(const QString &extension);
    FormattingEngine * formattingEngineReferenceAt(int index);
    QString defaultFormattingEngine() const;
    void registerLoggerEngineFactory(const QString &tag, LoggerFactoryInterface<LoggerEngine> *factory);
    QStringList availableLoggerEnginesInFactory() const;
    int attachedFormattingEngineCount() const;

    QStringList attachedLoggerEngineNames() const;
    int attachedLoggerEngineCount() const;
    LoggerEngine * loggerEngineReference(const QString &engineName);
    LoggerEngine * loggerEngineReferenceForFile(const QString &filePath);
    LoggerEngine * loggerEngineReferenceAt(int index);
    LoggerEngine * newLoggerEngine(QString tag, FormattingEngine *formattingEngine = 0);
    LoggerEngine * newFileEngine(const QString &engineName, const QString &fileName, const QString &formattingEngine = QString());
    void toggleQtMsgEngine(bool toggle);
    bool qtMsgEngineActive() const;
    void toggleConsoleEngine(bool toggle);
    bool consoleEngineActive() const;

public slots:
    void clear();
    void logMessage(const QString &engineName, MessageType type, const QVariant &message,
                    const QVariant &msg1 = QVariant(), const QVariant &msg2 = QVariant(), const QVariant &msg3 = QVariant(),
                    const QVariant &msg4 = QVariant(), const QVariant &msg5 = QVariant(), const QVariant &msg6 = QVariant(),
                    const QVariant &msg7 = QVariant(), const QVariant &msg8 = QVariant(), const QVariant &msg9 = QVariant());

signals:
    void newMessage(const QString &engineName, Logger::MessageType type, Logger::MessageContext context, const QList<QVariant> &contents);
    void newPriorityMessage(Logger::MessageType type, const QString &formattedMessage);

    void loggerEngineCountChanged(LoggerEngine *engine, Logger::EngineChangeIndication change);

private:
    Logger();

    static Logger * m_instance;

    LoggerFactory<LoggerEngine> m_loggerEngineFactory;
    QList<QPointer<LoggerEngine> > m_loggerEngines;
    QList<QPointer<FormattingEngine> > m_formattingEngines;
    QString m_defaultFormattingEngine;
    Logger::MessageType m_globalLogLevel;
    bool m_initialized;
    bool m_isQtMessageHandler;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Logger::MessageTypeFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Logger::MessageContextFlags)

Q_DECLARE_METATYPE(Logger::MessageType)


#define LOG Logger::instance()

#define LOG_INITIALIZE() LOG->initialize()
#define LOG_FINALIZE() LOG->finalize()

#ifndef QT_NO_DEBUG
#define LOG_TRACE(Msg, ...) LOG->logMessage(QString(), Logger::Trace, Msg, ##__VA_ARGS__)
#else
#define LOG_TRACE(Msg, ...) ((void)0)
#endif
#ifndef QT_NO_DEBUG
#define LOG_DEBUG(Msg, ...) LOG->logMessage(QString(), Logger::Debug, Msg, ##__VA_ARGS__)
#else
#define LOG_DEBUG(Msg, ...) ((void)0)
#endif
#define LOG_ERROR(Msg, ...) LOG->logMessage(QString(), Logger::Error, Msg, ##__VA_ARGS__)
#define LOG_WARNING(Msg, ...) LOG->logMessage(QString(), Logger::Warning, Msg, ##__VA_ARGS__)
#define LOG_FATAL(Msg, ...) LOG->logMessage(QString(), Logger::Fatal, Msg, ##__VA_ARGS__)
#define LOG_INFO(Msg, ...) LOG->logMessage(QString(), Logger::Info, Msg, ##__VA_ARGS__)

#endif // LOGGER_H
