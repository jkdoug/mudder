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
#include <QDebug>
#include <QDir>

QSettings Options::m_settings(QSettings::IniFormat, QSettings::UserScope, "Mudder");

Options::Options(QObject *parent) :
    QObject(parent)
{
    qDebug() << "options constructor";
}

QVariant Options::value(const QString &key, const QVariant &def)
{
    return m_settings.value(key, def);
}

void Options::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
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
}

QFont Options::loadFont(const QString &key, const QFont &def)
{
    return m_settings.value(key + "Font", def).value<QFont>();
}

void Options::saveFont(const QString &key, const QFont &font)
{
//    m_settings.setValue(key + "FontName", font.family());
//    m_settings.setValue(key + "FontSize", font.pointSize());
//    m_settings.setValue(key + "FontAntialias", font.styleStrategy() & QFont::PreferAntialias);

    m_settings.setValue(key + "Font", font);
}
