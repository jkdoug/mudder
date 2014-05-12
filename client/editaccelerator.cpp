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


#include "editaccelerator.h"
#include "ui_editaccelerator.h"
#include "luahighlighter.h"
#include "accelerator.h"
#include <QMessageBox>

EditAccelerator::EditAccelerator(QWidget *parent) :
    EditSetting(parent),
    ui(new Ui::EditAccelerator)
{
    ui->setupUi(this);

    connect(ui->name, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->key, SIGNAL(keySequenceChanged(QKeySequence)), SLOT(changed()));
}

EditAccelerator::~EditAccelerator()
{
    delete ui;
}

bool EditAccelerator::load(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Accelerator *accelerator = qobject_cast<Accelerator *>(item);
    Q_ASSERT(accelerator != 0);
    if (!accelerator)
    {
        return false;
    }

    m_key = accelerator->key();
    ui->key->setKeySequence(m_key);

    m_name = accelerator->name();
    ui->name->setText(m_name);

    m_enabled = accelerator->enabledFlag();
    ui->enabled->setChecked(m_enabled);

    m_contents = accelerator->contents();
    ui->script->setPlainText(m_contents);
    ui->script->setSyntaxHighlighter(new LuaHighlighter());

    ui->timesFired->setText(QLocale::system().toString(accelerator->executionCount()));
    ui->avgTime->setText(QString::number(accelerator->averageTime()));

    return true;
}

bool EditAccelerator::save(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Accelerator *accelerator = qobject_cast<Accelerator *>(item);
    Q_ASSERT(accelerator != 0);
    if (!accelerator)
    {
        return false;
    }

    QString name(ui->name->text());
    if (name.isEmpty())
    {
        QMessageBox::critical(this, tr("Invalid Accelerator"), tr("Name may not be left empty."));
        return false;
    }
    if (!ProfileItem::validateName(name))
    {
        QMessageBox::critical(this, tr("Invalid Accelerator"), tr("You may only use alphanumeric characters, underscores, and certain special characters in the name."));
        return false;
    }

    QString contents(ui->script->toPlainText().trimmed());
    if (contents.isEmpty())
    {
        QMessageBox::critical(this, tr("Invalid Accelerator"), tr("Script may not be left empty."));
        return false;
    }

    QKeySequence key(ui->key->keySequence());
    if (key.isEmpty() || key.count() > 1)
    {
        QMessageBox::critical(this, tr("Invalid Accelerator"), tr("You must specify exactly one key sequence to be matched."));
        return false;
    }

    m_key = key;
    accelerator->setKey(m_key);
    m_name = name;
    accelerator->setName(m_name);
    m_enabled = ui->enabled->isChecked();
    accelerator->enable(m_enabled);
    m_contents = ui->script->toPlainText().trimmed();
    accelerator->setContents(m_contents);

    accelerator->setFailed(false);

    emit itemModified(false, true);

    return true;
}

void EditAccelerator::changed()
{
    if (m_name.isNull())
    {
        emit itemModified(false, false);
        return;
    }

    QString script(ui->script->toPlainText().trimmed());

    bool changed = m_name != ui->name->text() ||
        m_key != ui->key->keySequence() ||
        m_enabled != ui->enabled->isChecked() ||
        m_contents != script;

    bool valid = !ui->name->text().isEmpty() &&
        !ui->key->keySequence().isEmpty();

    emit itemModified(changed, valid);
}
