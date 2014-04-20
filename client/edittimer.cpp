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


#include "edittimer.h"
#include "ui_edittimer.h"
#include "logger.h"
#include "luahighlighter.h"
#include "timer.h"

EditTimer::EditTimer(QWidget *parent) :
    EditSetting(parent),
    ui(new Ui::EditTimer)
{
    ui->setupUi(this);

    connect(ui->interval, SIGNAL(timeChanged(QTime)), SLOT(changed()));
    connect(ui->name, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->once, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), SLOT(changed()));
}

EditTimer::~EditTimer()
{
    delete ui;
}

bool EditTimer::load(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        LOG_ERROR("Attempted to load a timer from a null address.");
        return false;
    }

    LOG_TRACE("EditTimer::load", item->fullName());

    Timer *timer = qobject_cast<Timer *>(item);
    if (!timer)
    {
        LOG_ERROR("Attempted to load a timer from a non-timer item.");
        return false;
    }

    m_name = timer->name();
    ui->name->setText(m_name);

    m_enabled = timer->enabledFlag();
    ui->enabled->setChecked(m_enabled);

    m_interval = timer->interval();
    ui->interval->setTime(m_interval);

    m_once = timer->once();
    ui->once->setChecked(m_once);

    m_script = timer->contents();
    ui->script->setPlainText(m_script);
    ui->script->setSyntaxHighlighter(new LuaHighlighter());

    ui->timesFired->setText(QLocale::system().toString(timer->firedCount()));
    ui->avgTime->setText(QString::number(timer->averageTime()));

    return true;
}

bool EditTimer::save(ProfileItem *item)
{
    if (!item)
    {
        LOG_ERROR("Attempted to save a timer to a null address.");
        return false;
    }

    Timer *timer = qobject_cast<Timer *>(item);
    Q_ASSERT(timer != 0);
    if (!timer)
    {
        LOG_ERROR("Attempted to save a timer to a non-timer item.");
        return false;
    }

    LOG_TRACE("EditTimer::save", item->fullName());

    timer->setName(ui->name->text());

    timer->setInterval(ui->interval->time());

    timer->enable(ui->enabled->isChecked());
    timer->setOnce(ui->enabled->isChecked());

    timer->setContents(ui->script->toPlainText().trimmed());

    timer->setFailed(false);

    changed();

    return true;
}

void EditTimer::changed()
{
    if (m_name.isNull())
    {
        emit itemModified(false, false);
        return;
    }

    QString script(ui->script->toPlainText().trimmed());

    bool changed = m_name != ui->name->text() ||
            m_enabled != ui->enabled->isChecked() ||
            m_interval != ui->interval->time() ||
            m_once != ui->once->isChecked() ||
            m_script != script;

    bool valid = !ui->name->text().isEmpty() &&
            ui->interval->time().isValid() &&
            !script.isEmpty();

    emit itemModified(changed, valid);
}
