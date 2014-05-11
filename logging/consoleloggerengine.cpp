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


#include "consoleloggerengine.h"
#include <QFile>
#include <QMutex>

#define CONSOLE_RESET       "\033[0m"
#define CONSOLE_BLACK       "\033[30m"
#define CONSOLE_RED         "\033[31m"
#define CONSOLE_GREEN       "\033[32m"
#define CONSOLE_YELLOW      "\033[33m"
#define CONSOLE_BLUE        "\033[34m"
#define CONSOLE_MAGENTA     "\033[35m"
#define CONSOLE_CYAN        "\033[36m"
#define CONSOLE_WHITE       "\033[37m"
#define CONSOLE_BOLDBLACK   "\033[1m\033[30m"
#define CONSOLE_BOLDRED     "\033[1m\033[31m"
#define CONSOLE_BOLDGREEN   "\033[1m\033[32m"
#define CONSOLE_BOLDYELLOW  "\033[1m\033[33m"
#define CONSOLE_BOLDBLUE    "\033[1m\033[34m"
#define CONSOLE_BOLDMAGENTA "\033[1m\033[35m"
#define CONSOLE_BOLDCYAN    "\033[1m\033[36m"
#define CONSOLE_BOLDWHITE   "\033[1m\033[37m"

ConsoleLoggerEngine * ConsoleLoggerEngine::m_instance = 0;

ConsoleLoggerEngine * ConsoleLoggerEngine::instance()
{
    static QMutex mutex;
    if (!m_instance)
    {
        mutex.lock();
        if (!m_instance)
        {
            m_instance = new ConsoleLoggerEngine;
        }
        mutex.unlock();
    }

    return m_instance;
}

ConsoleLoggerEngine::ConsoleLoggerEngine() : LoggerEngine()
{
    setName("Console Logger Engine");
    m_formattingEnabled = true;

    m_colors[Logger::Warning] = QString(CONSOLE_BLUE);
    m_colors[Logger::Error] = QString(CONSOLE_RED);
    m_colors[Logger::Fatal] = QString(CONSOLE_RED);
}

ConsoleLoggerEngine::~ConsoleLoggerEngine()
{
}

void ConsoleLoggerEngine::setConsoleFormattingHint(Logger::MessageType type, const QString &color)
{
    m_colors[type] = color;
}

bool ConsoleLoggerEngine::initialize()
{
    m_initialized = true;
    return true;
}

void ConsoleLoggerEngine::finalize()
{
    m_initialized = false;
}

QString ConsoleLoggerEngine::description() const
{
    return "Writes log messages to the console.";
}

QString ConsoleLoggerEngine::status() const
{
    if (m_initialized)
    {
        if (m_active)
        {
            return "Logging in progress.";
        }
        return "Ready but inactive.";
    }
    return "Not initialized.";
}

void ConsoleLoggerEngine::resetConsoleEscapeCodes()
{
    QFile file;
    file.open(stdout, QIODevice::WriteOnly);
    file.write(CONSOLE_RESET);
    file.close();
}

void ConsoleLoggerEngine::logMessage(const QString &message, Logger::MessageType type)
{
    QFile file;
    QString logMessage(message + "\n");

#ifdef Q_OS_WIN
    if (type == Logger::Error || type == Logger::Fatal)
    {
        file.open(stderr, QIODevice::WriteOnly);
    }
    else
    {
        file.open(stdout, QIODevice::WriteOnly);
    }
#else
    if (m_formattingEnabled)
    {
        if (type == Logger::Info || type == Logger::Warning)
        {
            file.open(stdout, QIODevice::WriteOnly);

            if (m_colors.contains(type))
            {
                logMessage.prepend(m_colors[type]);
                logMessage.append(CONSOLE_RESET);
            }
            else
            {
                logMessage.prepend(CONSOLE_RESET);
            }
        }
        else if (type == Logger::Error || type == Logger::Fatal)
        {
            file.open(stderr, QIODevice::WriteOnly);

            logMessage.prepend(m_colors[type]);
            logMessage.append(CONSOLE_RESET);
        }
        else
        {
            file.open(stdout, QIODevice::WriteOnly);
        }
    }
    else
    {
        file.open(stdout, QIODevice::WriteOnly);
    }
#endif
    file.write(qPrintable(logMessage));
}
