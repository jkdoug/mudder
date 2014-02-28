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


#include "coresettings.h"
#include <QApplication>
#include <QDir>

CoreSettings::CoreSettings(QObject *parent) :
    QObject(parent),
    m_settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName())
{
}

CoreSettings * CoreSettings::instance()
{
    static CoreSettings instance;
    return &instance;
}

QVariant CoreSettings::value(const QString &key, const QVariant &def)
{
    if (def.isNull())
    {
        return m_settings.value(key, m_defaults.value(key));
    }

    return m_settings.value(key, def);
}

void CoreSettings::setValue(const QString &key, const QVariant &val)
{
    if (val == value(key))
    {
        return;
    }

    m_settings.setValue(key, val);
    emit valueChanged(key, val);
}

void CoreSettings::setDefault(const QString &key, const QVariant &val)
{
    m_defaults.insert(key, val);
}

QStringList CoreSettings::recentFileList()
{
    return value("RecentFileList").toStringList();
}

void CoreSettings::addRecentFile(const QString &fileName)
{
    QStringList files(recentFileList());

    files.removeAll(fileName);
    files.prepend(fileName);

    while (files.size() > MaxRecentFiles)
    {
        files.removeLast();
    }

    setValue("RecentFileList", files);

    emit recentFilesChanged(files);
}

void CoreSettings::removeRecentFile(const QString &fileName)
{
    QStringList files(recentFileList());

    files.removeAll(fileName);

    setValue("RecentFileList", files);

    emit recentFilesChanged(files);
}

QString CoreSettings::homePath() const
{
    return QDir::homePath() + QDir::separator() + QApplication::applicationName();
}
