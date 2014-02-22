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


#ifndef LOGGERENGINE_H
#define LOGGERENGINE_H

#include "logging_global.h"
#include "logger.h"
#include "formattingengine.h"
#include <QObject>

struct LoggerEnginePrivate
{
    LoggerEnginePrivate() :
        formattingEngine(0),
        isActive(true),
        isInitialized(false),
        isRemovable(true),
        messageContexts(Logger::AllMessageContexts)
    {}

    Logger::MessageTypeFlags    enabledMessageTypes;
    FormattingEngine *          formattingEngine;
    bool                        isActive;
    bool                        isInitialized;
    bool                        isRemovable;
    Logger::MessageContextFlags messageContexts;
    QString                     engineName;
};

class LOGGINGSHARED_EXPORT LoggerEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Logger::MessageTypeFlags enabledMessageTypes READ getEnabledMessageTypes WRITE setEnabledMessageTypes)
    Q_PROPERTY(QString formattingEngine READ formattingEngineName)

public:
    LoggerEngine();
    virtual ~LoggerEngine();

    virtual bool initialize() = 0;
    bool isInitialized() const;
    virtual void logMessage(const QString &message, Logger::MessageType type = Logger::Info) = 0;
    virtual void clearLog() {}

    bool isActive() const;
    void setActive(bool flag);

    QString name() const;
    void setName(const QString &name);
    virtual QString description() const = 0;
    virtual QString status() const = 0;
    virtual bool removable() const;
    virtual void setRemovable(bool flag);

    virtual void setEnabledMessageTypes(Logger::MessageTypeFlags types);
    virtual void enableAllMessageTypes();
    Logger::MessageTypeFlags getEnabledMessageTypes() const;

    void installFormattingEngine(FormattingEngine * engine);
    FormattingEngine* getInstalledFormattingEngine();
    QString formattingEngineName();

    virtual bool isFormattingEngineConstant() const = 0;

    inline Logger::MessageContextFlags messageContexts() const;
    void setMessageContexts(Logger::MessageContextFlags contexts);

public slots:
    virtual void finalize() = 0;
    virtual void newMessages(const QString &engineName, Logger::MessageType type, Logger::MessageContext context, const QList<QVariant> &messages);

protected:
    LoggerEnginePrivate * m_d;
};

#endif // LOGGERENGINE_H
