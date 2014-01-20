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


#include <QSettings>
#include "dialogglobal.h"
#include "ui_dialogglobal.h"

DialogGlobal::DialogGlobal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGlobal)
{
    ui->setupUi(this);

    QStringList fontSizes;
    for (int n = 6; n <= 30; n++)
    {
        fontSizes << QString::number(n);
    }
    ui->comboEditorFontSize->addItems(fontSizes);

    load();
}

DialogGlobal::~DialogGlobal()
{
    delete ui;
}

void DialogGlobal::accept()
{
    if (validate())
    {
        save();

        QDialog::accept();
    }
}

void DialogGlobal::load()
{
    QSettings settings;

    bool antiAlias = settings.value("GlobalFontAntialias", true).toBool();
    QFont font(settings.value("GlobalFontName", "Consolas").toString(), settings.value("GlobalFontSize", 9).toInt());
    font.setStyleHint(QFont::TypeWriter);
    ui->comboEditorFont->setCurrentFont(font);
    ui->comboEditorFontSize->setCurrentIndex(font.pointSize() - 6);
    ui->checkEditorAntiAliased->setChecked(antiAlias);
}

void DialogGlobal::save()
{
    bool antiAlias = ui->checkEditorAntiAliased->isChecked();
    m_editorFont = ui->comboEditorFont->currentFont();
    m_editorFont.setStyleHint(QFont::TypeWriter, antiAlias?QFont::PreferAntialias:QFont::NoAntialias);
    m_editorFont.setPointSize(ui->comboEditorFontSize->currentText().toInt());

    QSettings settings;

    settings.setValue("GlobalFontName", m_editorFont.family());
    settings.setValue("GlobalFontSize", m_editorFont.pointSize());
    settings.setValue("GlobalFontAntialias", antiAlias);
}

bool DialogGlobal::validate()
{
    return true;
}

void DialogGlobal::on_pageList_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}
