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
#include "editvariable.h"
#include "ui_editvariable.h"
#include "variable.h"

EditVariable::EditVariable(QWidget *parent) :
    EditSetting(parent),
    ui(new Ui::EditVariable)
{
    ui->setupUi(this);

    ui->type->addItems(QStringList() << "string" << "numeric" << "boolean");

    connect(ui->name, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->contents, SIGNAL(textChanged()), SLOT(changed()));
}

EditVariable::~EditVariable()
{
    delete ui;
}

bool EditVariable::load(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Variable *variable = qobject_cast<Variable *>(item);
    Q_ASSERT(variable != 0);
    if (!variable)
    {
        return false;
    }

    m_name = item->name();
    ui->name->setText(m_name);

    m_contents = variable->contents();
    switch (m_contents.type())
    {
        case QVariant::Bool:
            ui->type->setCurrentIndex(2);
            break;

        case QVariant::Double:
            ui->type->setCurrentIndex(1);
            break;

        default:
            ui->type->setCurrentIndex(0);
            break;
    }

    ui->contents->setPlainText(m_contents.toString());

    changed();

    return true;
}

bool EditVariable::save(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Variable *variable = qobject_cast<Variable *>(item);
    Q_ASSERT(variable != 0);
    if (!variable)
    {
        return false;
    }

    QString name(ui->name->text());
    if (!Variable::validateName(name))
    {
        QMessageBox::critical(this, tr("Invalid Variable"), tr("You may only use alphanumeric characters and underscores in variable names."));
        return false;
    }

    QString type(ui->type->itemText(ui->type->currentIndex()));
    QVariant::Type enumType = Variable::translateType(type);

    QVariant val(ui->contents->toPlainText().trimmed());
    if (!val.convert(enumType))
    {
        QMessageBox::critical(this, tr("Invalid Variable"), tr("The selected data type does not allow this data to be stored."));
        return false;
    }

    variable->setName(name);

    variable->setContents(val);

    changed();

    return true;
}

void EditVariable::changed()
{
    if (m_name.isNull())
    {
        emit itemModified(false, false);
        return;
    }

    bool changed = m_name != ui->name->text() ||
        m_contents.toString() != ui->contents->toPlainText();

    QString type(ui->type->itemText(ui->type->currentIndex()));
    QVariant::Type enumType = Variable::translateType(type);

    QVariant val(ui->contents->toPlainText().trimmed());
    bool valid = !ui->name->text().isEmpty() && val.convert(enumType);

    emit itemModified(changed, valid);
}
