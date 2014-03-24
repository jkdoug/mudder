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


#include "consoleloggerengine.h"
#include <QMutex>

#ifndef Q_OS_WIN
#include <stdio.h>
#endif

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
    m_d->isInitialized = true;
    return true;
}

void ConsoleLoggerEngine::finalize()
{
    m_d->isInitialized = false;
}

QString ConsoleLoggerEngine::description() const
{
    return "Writes log messages to the console.";
}

QString ConsoleLoggerEngine::status() const
{
    if (m_d->isInitialized)
    {
        if (m_d->isActive)
        {
            return "Logging in progress.";
        }
        return "Ready but inactive.";
    }
    return "Not initialized.";
}

void ConsoleLoggerEngine::resetConsoleEscapeCodes()
{
#ifndef Q_OS_WIN
    fprintf(stdout, CONSOLE_RESET);
#endif
}

void ConsoleLoggerEngine::logMessage(const QString &message, Logger::MessageType type)
{
#ifdef Q_OS_WIN
    if (type == Logger::Error || type == Logger::Fatal)
    {
        fprintf(stderr, "%s\n", qPrintable(message));
    }
    else
    {
        fprintf(stdout, "%s\n", qPrintable(message));
    }
#else
    if (m_formattingEnabled)
    {
        if (type == Logger::Info)
        {
            if (m_colors.contains(Logger::Info))
            {
                fprintf(stdout, qPrintable(QString("%1%s\n%2").arg(m_colors[Logger::Info]).arg(CONSOLE_RESET)), qPrintable(message));
            }
            else
            {
                fprintf(stdout, qPrintable(QString("%1%s\n").arg(CONSOLE_RESET)), qPrintable(message));
            }
        } else if (type == Logger::Warning)
        {
            fprintf(stdout, qPrintable(QString("%1%s\n%2").arg(m_colors[Logger::Warning]).arg(CONSOLE_RESET)), qPrintable(message));
        }
        else if (type == Logger::Error)
        {
            fprintf(stderr, qPrintable(QString("%1%s\n%2").arg(m_colors[Logger::Error]).arg(CONSOLE_RESET)), qPrintable(message));
        }
        else if (type == Logger::Fatal)
        {
            fprintf(stderr, qPrintable(QString("%1%s\n%2").arg(m_colors[Logger::Fatal]).arg(CONSOLE_RESET)), qPrintable(message));
        }
        else
        {
            fprintf(stdout, "%s\n", qPrintable(message));
        }
    }
    else
    {
        fprintf(stdout, "%s\n", qPrintable(message));
    }
#endif
}
