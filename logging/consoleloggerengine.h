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


#ifndef CONSOLELOGGERENGINE_H
#define CONSOLELOGGERENGINE_H

#include "logging_global.h"
#include "loggerengine.h"

class LOGGINGSHARED_EXPORT ConsoleLoggerEngine : public LoggerEngine
{
    Q_OBJECT

public:
    static ConsoleLoggerEngine * instance();
    ~ConsoleLoggerEngine();

protected:
    ConsoleLoggerEngine();

public:
    void setConsoleFormattingEnabled(bool flag) { m_formattingEnabled = flag; }
    bool consoleFormattingEnabled() const { return m_formattingEnabled; }
    void setConsoleFormattingHint(Logger::MessageType type, const QString &color);
    void resetConsoleEscapeCodes();

    bool initialize();
    void finalize();
    QString description() const;
    QString status() const;
    bool removable() const { return false; }
    bool isFormattingEngineConstant() const { return false; }

public slots:
    void logMessage(const QString &message, Logger::MessageType type);

private:
    static ConsoleLoggerEngine * m_instance;
    bool m_formattingEnabled;
    QMap<Logger::MessageType, QString> m_colors;
};

#endif // CONSOLELOGGERENGINE_H
