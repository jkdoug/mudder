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

    bool isActive() const { return m_active; }
    void setActive(bool flag) { m_active = flag; }

    QString name() const { return m_engineName; }
    void setName(const QString &name);
    virtual QString description() const = 0;
    virtual QString status() const = 0;
    virtual bool removable() const { return m_removable; }
    virtual void setRemovable(bool flag) { m_removable = flag; }

    virtual void setEnabledMessageTypes(Logger::MessageTypeFlags types) { m_enabledMessageTypes = types; }
    Logger::MessageTypeFlags getEnabledMessageTypes() const { return m_enabledMessageTypes; }
    virtual void enableAllMessageTypes();

    void installFormattingEngine(FormattingEngine * engine);
    FormattingEngine* getInstalledFormattingEngine() { return m_formattingEngine; }
    QString formattingEngineName();

    virtual bool isFormattingEngineConstant() const = 0;

    inline Logger::MessageContextFlags messageContexts() const { return m_messageContexts; }
    void setMessageContexts(Logger::MessageContextFlags contexts) { m_messageContexts = contexts; }

public slots:
    virtual void finalize() = 0;
    virtual void newMessages(const QString &engineName, Logger::MessageType type, Logger::MessageContext context, const QList<QVariant> &messages);

protected:
    Logger::MessageTypeFlags m_enabledMessageTypes;
    FormattingEngine *m_formattingEngine;
    bool m_active;
    bool m_initialized;
    bool m_removable;
    Logger::MessageContextFlags m_messageContexts;
    QString m_engineName;
};

#endif // LOGGERENGINE_H
