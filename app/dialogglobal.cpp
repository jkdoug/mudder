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
#include "options.h"

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
    QFont font(Options::editorFont());

    ui->comboEditorFont->setCurrentFont(font);
    ui->comboEditorFontSize->setCurrentIndex(font.pointSize() - 6);
    ui->checkEditorAntiAliased->setChecked(font.styleStrategy() & QFont::PreferAntialias);
}

void DialogGlobal::save()
{
    bool antiAlias = ui->checkEditorAntiAliased->isChecked();

    QFont font(ui->comboEditorFont->currentFont());
    font.setStyleHint(QFont::TypeWriter, antiAlias?QFont::PreferAntialias:QFont::NoAntialias);
    font.setPointSize(ui->comboEditorFontSize->currentText().toInt());

    Options::setEditorFont(font);
}

bool DialogGlobal::validate()
{
    return true;
}

void DialogGlobal::on_pageList_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}
