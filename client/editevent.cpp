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


#include "editevent.h"
#include "ui_editevent.h"
#include "codeeditor.h"
#include "luahighlighter.h"
#include "event.h"
#include <QMessageBox>

EditEvent::EditEvent(QWidget *parent) :
    EditSetting(parent),
    ui(new Ui::EditEvent)
{
    ui->setupUi(this);

    ui->script->setSyntaxHighlighter(new LuaHighlighter());

    connect(ui->name, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->title, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->sequence, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), SLOT(changed()));
}

EditEvent::~EditEvent()
{
    delete ui;
}

bool EditEvent::load(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Event *event = qobject_cast<Event *>(item);
    Q_ASSERT(event != 0);
    if (!event)
    {
        return false;
    }

    m_name = event->name();
    ui->name->setText(m_name);

    m_title = event->title();
    ui->title->setText(m_title);

    m_sequence = event->sequence();
    ui->sequence->setValue(m_sequence);

    m_enabled = event->enabledFlag();
    ui->enabled->setChecked(m_enabled);

    m_contents = event->contents();
    ui->script->setPlainText(m_contents);

    ui->timesEvaluated->setText(QLocale::system().toString(event->evalCount()));
    ui->timesFired->setText(QLocale::system().toString(event->matchCount()));
    if (event->matchCount() > 0)
    {
        ui->lastFired->setText(event->lastMatched().toString(Qt::SystemLocaleShortDate));
    }
    else
    {
        ui->lastFired->setText(tr("Never"));
    }
    ui->avgTime->setText(QString::number(event->averageTime()));

    changed();

    return true;
}

bool EditEvent::save(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Event *event = qobject_cast<Event *>(item);
    Q_ASSERT(event != 0);
    if (!event)
    {
        return false;
    }

    QString name(ui->name->text());
    if (name.isEmpty())
    {
        QMessageBox::critical(this, tr("Invalid Event"), tr("Name may not be left empty."));
        return false;
    }
    if (!ProfileItem::validateName(name))
    {
        QMessageBox::critical(this, tr("Invalid Event"), tr("You may only use alphanumeric characters, underscores, and certain special characters in the name."));
        return false;
    }

    QString contents(ui->script->toPlainText().trimmed());
    if (contents.isEmpty())
    {
        QMessageBox::critical(this, tr("Invalid Event"), tr("Script may not be left empty."));
        return false;
    }

    QString title(ui->title->text());
    if (title.isEmpty())
    {
        QMessageBox::critical(this, tr("Invalid Event"), tr("Title may not be left empty."));
        return false;
    }

    m_name = name;
    event->setName(m_name);

    m_title = title;
    event->setTitle(m_title);

    m_sequence = ui->sequence->value();
    event->setSequence(m_sequence);

    m_enabled = ui->enabled->isChecked();
    event->enable(m_enabled);

    m_contents = contents;
    event->setContents(m_contents);

    event->setFailed(false);

    emit itemModified(false, true);

    return true;
}

void EditEvent::changed()
{
    if (m_name.isNull())
    {
        emit itemModified(false, false);
        return;
    }

    QString script(ui->script->toPlainText().trimmed());

    bool changed = m_name != ui->name->text() ||
        m_title != ui->title->text() ||
        m_sequence != ui->sequence->value() ||
        m_enabled != ui->enabled->isChecked() ||
        m_contents != script;

    bool valid = !ui->name->text().isEmpty() &&
        !ui->title->text().isEmpty() &&
        !script.isEmpty();

    emit itemModified(changed, valid);
}
