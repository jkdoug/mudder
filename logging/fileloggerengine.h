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


#ifndef FILELOGGERENGINE_H
#define FILELOGGERENGINE_H

#include "logging_global.h"
#include "loggerengine.h"
#include "loggerexportable.h"

class LOGGINGSHARED_EXPORT FileLoggerEngine : public LoggerEngine, public LoggerExportable
{
    Q_OBJECT
    Q_INTERFACES(LoggerExportable)
    Q_PROPERTY(QString fileName READ fileName)

public:
    FileLoggerEngine();
    ~FileLoggerEngine();

    bool initialize();
    void finalize();
    QString description() const;
    QString status() const;
    bool isFormattingEngineConstant() const { return true; }
    void clearLog();

    ExportModeFlags supportedFormats() const;
    bool exportBinary(QDataStream &stream) const;
    bool importBinary(QDataStream &stream);
    QString factoryTag() const { return "def.FactoryTag.File"; }
    QString instanceName() const { return name(); }

    void setFileName(const QString &fileName);
    QString fileName() const { return m_fileName; }

    static LoggerFactoryItem<LoggerEngine, FileLoggerEngine> factory;

public slots:
    void logMessage(const QString &message, Logger::MessageType type);

private:
    QString m_fileName;
};

#endif // FILELOGGERENGINE_H
