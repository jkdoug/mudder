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


#include <QMessageBox>
#include "editalias.h"
#include "ui_editalias.h"
#include "codeeditor.h"
#include "luahighlighter.h"
#include "logger.h"
#include "alias.h"

EditAlias::EditAlias(QWidget *parent) :
    EditSetting(parent),
    ui(new Ui::EditAlias)
{
    ui->setupUi(this);

    connect(ui->name, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->pattern, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->sequence, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), SLOT(changed()));
    connect(ui->caseSensitive, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->keepEvaluating, SIGNAL(stateChanged(int)), SLOT(changed()));
}

EditAlias::~EditAlias()
{
    delete ui;
}

bool EditAlias::load(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        LOG_ERROR("Attempted to load an alias from a null address.");
        return false;
    }

    LOG_TRACE("EditAlias::load", item->fullName());

    Alias *alias = qobject_cast<Alias *>(item);
    if (!alias)
    {
        LOG_ERROR("Attempted to load an alias from a non-alias item.");
        return false;
    }

    m_name = alias->name();
    ui->name->setText(m_name);

    m_pattern = alias->pattern();
    ui->pattern->setText(m_pattern);

    m_sequence = alias->sequence();
    ui->sequence->setValue(m_sequence);

    m_enabled = alias->enabledFlag();
    ui->enabled->setChecked(m_enabled);

    m_caseSensitive = alias->caseSensitive();
    ui->caseSensitive->setChecked(m_caseSensitive);

    m_keepEvaluating = alias->keepEvaluating();
    ui->keepEvaluating->setChecked(m_keepEvaluating);

    m_script = alias->contents();
    ui->script->setPlainText(m_script);
    ui->script->setSyntaxHighlighter(new LuaHighlighter());

    ui->timesEvaluated->setText(QLocale::system().toString(alias->evalCount()));
    ui->timesFired->setText(QLocale::system().toString(alias->matchCount()));
    if (alias->matchCount() > 0)
    {
        ui->lastFired->setText(alias->lastMatched().toString(Qt::SystemLocaleShortDate));
    }
    else
    {
        ui->lastFired->setText(tr("Never"));
    }
    ui->avgTime->setText(QString::number(alias->averageTime()));

    changed();

    return true;
}

bool EditAlias::save(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        LOG_ERROR("Attempted to save an alias to a null address.");
        return false;
    }

    Alias *alias = qobject_cast<Alias *>(item);
    Q_ASSERT(alias != 0);
    if (!alias)
    {
        LOG_ERROR("Attempted to save an alias to a non-alias item.");
        return false;
    }

    LOG_TRACE("EditAlias::save", item->fullName());

    QString name(ui->name->text());
    if (!ProfileItem::validateName(name))
    {
        QMessageBox::critical(this, tr("Invalid Alias"), tr("You may only use alphanumeric characters, underscores, and certain special characters in the name."));
        return false;
    }

    alias->setName(name);

    alias->setPattern(ui->pattern->text());

    alias->setName(ui->name->text());
    alias->setSequence(ui->sequence->value());

    alias->enable(ui->enabled->isChecked());
    alias->setKeepEvaluating(ui->keepEvaluating->isChecked());
    alias->setCaseSensitive(ui->caseSensitive->isChecked());

    alias->setContents(ui->script->toPlainText().trimmed());

    alias->setFailed(false);

    changed();

    return true;
}

void EditAlias::changed()
{
    if (m_name.isNull())
    {
        emit itemModified(false, false);
        return;
    }

    QString script(ui->script->toPlainText().trimmed());

    bool changed = m_name != ui->name->text() ||
            m_pattern != ui->pattern->text() ||
            m_sequence != ui->sequence->value() ||
            m_enabled != ui->enabled->isChecked() ||
            m_keepEvaluating != ui->keepEvaluating->isChecked() ||
            m_caseSensitive != ui->caseSensitive->isChecked() ||
            m_script != script;

    bool valid = !ui->name->text().isEmpty() &&
            !script.isEmpty();

    emit itemModified(changed, valid);
}
