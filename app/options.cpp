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


#include "options.h"
#include <QApplication>
#include <QDebug>
#include <QDir>

Options::Options(QObject *parent) :
    QObject(parent),
    m_settings(QSettings::IniFormat, QSettings::UserScope, "Mudder")
{
}

Options * Options::P()
{
    static Options instance;
    return &instance;
}

QVariant Options::value(const QString &key, const QVariant &def)
{
    return m_settings.value(key, def);
}

void Options::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);

    emit valueChanged(key, value);
}


QFont Options::editorFont()
{
    QFont font("Consolas", 9);
    font.setStyleHint(QFont::TypeWriter);
    font.setStyleStrategy(QFont::PreferAntialias);

    return loadFont("Editor", font);
}

void Options::setEditorFont(const QFont &font)
{
    saveFont("Editor", font);

    emit editorFontChanged(font);
}

QString Options::homePath()
{
    return QDir::homePath() + QDir::separator() + QApplication::applicationName();
}

QStringList Options::recentFileList()
{
    return value("RecentFileList").toStringList();
}

void Options::addRecentFile(const QString &fileName)
{
    QStringList files(recentFileList());

    files.removeAll(fileName);
    files.prepend(fileName);

    while (files.size() > Options::MaxRecentFiles)
    {
        files.removeLast();
    }

    setValue("RecentFileList", files);

    emit recentFilesChanged(files);
}

void Options::removeRecentFile(const QString &fileName)
{
    QStringList files(recentFileList());

    files.removeAll(fileName);

    setValue("RecentFileList", files);

    emit recentFilesChanged(files);
}

QFont Options::loadFont(const QString &key, const QFont &def)
{
    QFont font(m_settings.value(key + "Font", def).value<QFont>());

    return font;
}

void Options::saveFont(const QString &key, const QFont &font)
{
    m_settings.setValue(key + "Font", font);
}
