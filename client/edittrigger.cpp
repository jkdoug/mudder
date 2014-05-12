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


#include <QMessageBox>
#include "edittrigger.h"
#include "ui_edittrigger.h"
#include "luahighlighter.h"
#include "trigger.h"

EditTrigger::EditTrigger(QWidget *parent) :
    EditSetting(parent),
    ui(new Ui::EditTrigger)
{
    ui->setupUi(this);

    connect(ui->name, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->pattern, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->sequence, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), SLOT(changed()));
    connect(ui->caseSensitive, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->keepEvaluating, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->repeat, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->omit, SIGNAL(stateChanged(int)), SLOT(changed()));
}

EditTrigger::~EditTrigger()
{
    delete ui;
}

bool EditTrigger::load(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Trigger *trigger = qobject_cast<Trigger *>(item);
    Q_ASSERT(trigger != 0);
    if (!trigger)
    {
        return false;
    }

    m_name = trigger->name();
    ui->name->setText(m_name);

    m_pattern = trigger->pattern();
    ui->pattern->setText(m_pattern);

    m_sequence = trigger->sequence();
    ui->sequence->setValue(m_sequence);

    m_enabled = trigger->enabledFlag();
    ui->enabled->setChecked(m_enabled);

    m_caseSensitive = trigger->caseSensitive();
    ui->caseSensitive->setChecked(m_caseSensitive);

    m_keepEvaluating = trigger->keepEvaluating();
    ui->keepEvaluating->setChecked(m_keepEvaluating);

    m_repeat = trigger->repeat();
    ui->repeat->setChecked(m_repeat);

    m_omit = trigger->omit();
    ui->omit->setChecked(m_omit);

    m_contents = trigger->contents();
    ui->script->setPlainText(m_contents);
    ui->script->setSyntaxHighlighter(new LuaHighlighter());

    ui->timesEvaluated->setText(QLocale::system().toString(trigger->evalCount()));
    ui->timesFired->setText(QLocale::system().toString(trigger->matchCount()));
    if (trigger->matchCount() > 0)
    {
        ui->lastFired->setText(trigger->lastMatched().toString(Qt::SystemLocaleShortDate));
    }
    else
    {
        ui->lastFired->setText(tr("Never"));
    }
    ui->avgTime->setText(QString::number(trigger->averageTime()));

    changed();

    return true;
}

bool EditTrigger::save(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Trigger *trigger = qobject_cast<Trigger *>(item);
    Q_ASSERT(trigger != 0);
    if (!trigger)
    {
        return false;
    }

    QString name(ui->name->text());
    if (!ProfileItem::validateName(name))
    {
        QMessageBox::critical(this, tr("Invalid Trigger"), tr("You may only use alphanumeric characters, underscores, and certain special characters in the name."));
        return false;
    }

    QString contents(ui->script->toPlainText().trimmed());
    bool omit = ui->omit->isChecked();
    if (contents.isEmpty() && !omit)
    {
        QMessageBox::critical(this, tr("Invalid Trigger"), tr("Script may not be left empty."));
        return false;
    }

    QString pattern(ui->pattern->text());
    QRegularExpression regex(pattern);
    if (!regex.isValid())
    {
        QMessageBox::critical(this, tr("Invalid Trigger"), tr("Invalid regular expression: %1").arg(regex.errorString()));
        return false;
    }

    m_name = name;
    trigger->setName(m_name);

    m_pattern = pattern;
    trigger->setPattern(m_pattern);

    m_sequence = ui->sequence->value();
    trigger->setSequence(m_sequence);

    m_enabled = ui->enabled->isChecked();
    trigger->enable(m_enabled);
    m_keepEvaluating = ui->keepEvaluating->isChecked();
    trigger->setKeepEvaluating(m_keepEvaluating);
    m_caseSensitive = ui->caseSensitive->isChecked();
    trigger->setCaseSensitive(m_caseSensitive);
    m_repeat = ui->repeat->isChecked();
    trigger->setRepeat(m_repeat);
    m_omit = omit;
    trigger->setOmit(m_omit);

    m_contents = contents;
    trigger->setContents(m_contents);

    trigger->setFailed(false);

    emit itemModified(false, true);

    return true;
}

void EditTrigger::changed()
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
        m_repeat != ui->repeat->isChecked() ||
        m_omit != ui->omit->isChecked() ||
        m_contents != script;

    bool valid = !ui->name->text().isEmpty() &&
        (!script.isEmpty() || ui->omit->isChecked());

    emit itemModified(changed, valid);
}
