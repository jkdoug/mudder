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


#include "fileloggerengine.h"
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QtDebug>

LoggerFactoryItem<LoggerEngine, FileLoggerEngine> FileLoggerEngine::factory;

FileLoggerEngine::FileLoggerEngine() :
    LoggerEngine()
{
    setName("File Logger Engine");
}

FileLoggerEngine::~FileLoggerEngine()
{
    finalize();
}

bool FileLoggerEngine::initialize()
{
    if (m_fileName.isEmpty())
    {
//        LOG_ERROR(QString("Failed to initialize file logger engine (%1): File name is empty...").arg(objectName()));
        return false;
    }

    if (!m_d->formattingEngine)
    {
        QFileInfo fileInfo(m_fileName);
        FormattingEngine* formattingEngine = LOG->formattingEngineReferenceFromExtension(fileInfo.suffix());
        if (!formattingEngine)
        {
            m_d->formattingEngine = LOG->formattingEngineReference("Default");
//            LOG_DEBUG(QString("Assigning default formatting engine to file logger engine (%1).").arg(objectName()));
        }
        else
        {
            m_d->formattingEngine = formattingEngine;
        }
    }

    QFileInfo fileInfo(m_fileName);
    QDir dir(fileInfo.path());
    if (!dir.exists())
    {
        dir.mkpath(fileInfo.path());
    }

    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
//        LOG_ERROR(QString("Failed to initialize file logger engine (%1): Can't open the specified file (%2) for writing.").arg(objectName()).arg(m_fileName));
        return false;
    }

    QTextStream out(&file);
    out << m_d->formattingEngine->initializeString();
    file.close();

    m_d->isInitialized = true;
    return true;
}

void FileLoggerEngine::finalize()
{
    if (m_d->isInitialized)
    {
        QFile file(m_fileName);
        if (!file.exists())
        {
            return;
        }

        if (!file.open(QIODevice::Append | QIODevice::Text))
        {
            return;
        }

        if (m_d->formattingEngine)
        {
            QTextStream out(&file);
            out << m_d->formattingEngine->finalizeString();
            file.close();
        }
    }
}

QString FileLoggerEngine::description() const
{
    return "Writes log messages to a file.";
}

QString FileLoggerEngine::status() const
{
    if (m_d->isInitialized)
    {
        if (m_d->isActive)
        {
            return QString("Logging to output file: %1").arg(m_fileName);
        }
        return "Ready but inactive.";
    }
    return "Not initialized.";
}

void FileLoggerEngine::clearLog()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to clear file logger engine:" << m_fileName;
        return;
    }
    file.close();
}

void FileLoggerEngine::logMessage(const QString &message, Logger::MessageType type)
{
    Q_UNUSED(type)

    if (!m_d->isInitialized)
    {
        return;
    }

    QFile file(m_fileName);
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        return;
    }

    QTextStream out(&file);
    out << message;
    file.close();
}


LoggerExportable::ExportModeFlags FileLoggerEngine::supportedFormats() const
{
    return LoggerExportable::Binary;
}

bool FileLoggerEngine::exportBinary(QDataStream &stream) const
{
    stream << m_fileName;
    return true;
}

bool FileLoggerEngine::importBinary(QDataStream &stream)
{
    stream >> m_fileName;
    return true;
}

void FileLoggerEngine::setFileName(const QString &fileName)
{
    if (!m_d->isInitialized)
    {
        m_fileName = fileName;
    }
}
