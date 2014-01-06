/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "dialogprofile.h"
#include "ui_dialogprofile.h"
#include "profile.h"
#include "console.h"

#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

DialogProfile::DialogProfile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProfile)
{
    ui->setupUi(this);

    // TODO: show if/when text wrap is customizable again
    ui->groupDisplayOptions->setVisible(false);

    ui->stackedWidget->setCurrentIndex(ui->pageList->currentRow());

    for (int n = 0; n < ui->pageList->count(); n++)
    {
        QListWidgetItem *item = ui->pageList->item(n);
        item->setTextAlignment(Qt::AlignCenter);
    }

    QStringList fontSizes;
    for (int n = 6; n <= 30; n++)
    {
        fontSizes << QString::number(n);
    }
    ui->comboOutputFontSize->addItems(fontSizes);

    fontSizes.clear();
    for (int n = 6; n <= 20; n++)
    {
        fontSizes << QString::number(n);
    }
    ui->comboInputFontSize->addItems(fontSizes);

    m_profile = new Profile(this);
}

DialogProfile::~DialogProfile()
{
    delete ui;
}

void DialogProfile::load(Profile *profile)
{
    Q_ASSERT(profile != 0);
    Q_ASSERT(m_profile != 0);

    m_profile->copyPreferences(*profile);

    load();
}

void DialogProfile::accept()
{
    if (validate())
    {
        save();

        QDialog::accept();
    }
}

void DialogProfile::load()
{
    Q_ASSERT(m_profile != 0);

    ui->name->setText(m_profile->name());
    ui->address->setText(m_profile->address());
    ui->port->setText(QString::number(m_profile->port()));

    ui->checkAutoConnect->setChecked(m_profile->autoConnect());
    ui->checkReconnect->setChecked(m_profile->autoReconnect());
    ui->checkClearCommands->setChecked(m_profile->clearCommandAfterSend());

    ui->scriptFilename->setText(m_profile->scriptFilename());
    ui->scriptPrefix->setText(m_profile->scriptPrefix());

    QFont fontOutput(m_profile->outputFont());
    ui->comboOutputFont->setCurrentFont(fontOutput);
    ui->comboOutputFontSize->setCurrentIndex(fontOutput.pointSize() - 6);
    ui->checkOutputAntiAliased->setChecked(m_profile->isOutputAntiAliased());

    QFont fontInput(m_profile->inputFont());
    ui->comboInputFont->setCurrentFont(fontInput);
    ui->comboInputFontSize->setCurrentIndex(fontInput.pointSize() - 6);
    ui->checkInputAntiAliased->setChecked(m_profile->isInputAntiAliased());

    ui->checkAutoWrap->setChecked(m_profile->autoWrap());
    ui->wrapColumn->setEnabled(!m_profile->autoWrap());
    ui->wrapColumn->setValue(m_profile->wrapColumn());

    m_fgCommand = m_profile->commandForeground();
    m_bgCommand = m_profile->commandBackground();
    ui->fgCommand->setStyleSheet(QString("QPushButton { background-color: %1; }")
                                 .arg(m_fgCommand.name()));
    ui->bgCommand->setStyleSheet(QString("QPushButton { background-color: %1; }")
                                 .arg(m_bgCommand.name()));
    ui->editCommandSample->setStyleSheet(QString("color: %1; background-color: %2;")
                                         .arg(m_fgCommand.name())
                                         .arg(m_bgCommand.name()));
    m_fgNote = m_profile->noteForeground();
    m_bgNote = m_profile->noteBackground();
    ui->fgNote->setStyleSheet(QString("QPushButton { background-color: %1; }")
                              .arg(m_fgNote.name()));
    ui->bgNote->setStyleSheet(QString("QPushButton { background-color: %1; }")
                              .arg(m_bgNote.name()));
    ui->editNoteSample->setStyleSheet(QString("color: %1; background-color: %2;")
                                      .arg(m_fgNote.name())
                                      .arg(m_bgNote.name()));

    ui->loggingDirectory->setText(m_profile->logDirectory());
    ui->loggingFilename->setText(m_profile->logFilename());
    ui->loggingAutoStart->setChecked(m_profile->autoLog());
}

void DialogProfile::save()
{
    Q_ASSERT(m_profile != 0);

    m_profile->setName(ui->name->text());
    m_profile->setAddress(ui->address->text());
    m_profile->setPort(ui->port->text().toInt());

    m_profile->setAutoConnect(ui->checkAutoConnect->isChecked());
    m_profile->setAutoReconnect(ui->checkReconnect->isChecked());
    m_profile->setClearCommandAfterSend(ui->checkClearCommands->isChecked());

    m_profile->setScriptFilename(ui->scriptFilename->text());
    m_profile->setScriptPrefix(ui->scriptPrefix->text());

    QFont fontOutput(ui->comboOutputFont->currentFont());
    fontOutput.setPointSize(ui->comboOutputFontSize->currentText().toInt());
    m_profile->setOutputFont(fontOutput, ui->checkOutputAntiAliased->isChecked());

    QFont fontInput(ui->comboInputFont->currentFont());
    fontInput.setPointSize(ui->comboInputFontSize->currentText().toInt());
    m_profile->setInputFont(fontInput, ui->checkInputAntiAliased->isChecked());

    m_profile->setAutoWrap(ui->checkAutoWrap->isChecked());
    m_profile->setWrapColumn(ui->wrapColumn->value());

    m_profile->setCommandForeground(m_fgCommand);
    m_profile->setCommandBackground(m_bgCommand);
    m_profile->setNoteForeground(m_fgNote);
    m_profile->setNoteBackground(m_bgNote);

    m_profile->setLogDirectory(ui->loggingDirectory->text());
    m_profile->setLogFilename(ui->loggingFilename->text());
    m_profile->setAutoLog(ui->loggingAutoStart->isChecked());
}

bool DialogProfile::validate()
{
    if (ui->name->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid Entry"), tr("The profile name must not be empty."));
        return false;
    }

    if (ui->address->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid Entry"), tr("The host address must not be empty."));
        return false;
    }

    bool valid = false;
    int port = ui->port->text().toInt(&valid);
    if (!valid || port < 1 || port > 65535)
    {
        QMessageBox::warning(this, tr("Invalid Entry"), tr("The host port must be a valid number between 1 and 65535."));
        return false;
    }

    if (ui->loggingAutoStart->isChecked())
    {
        if (ui->loggingDirectory->text().trimmed().isEmpty() || ui->loggingFilename->text().trimmed().isEmpty())
        {
            QMessageBox::warning(this, tr("Invalid Entry"), tr("The logging directory and filename must not be empty."));
            return false;
        }
    }

    return true;
}

void DialogProfile::on_pageList_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void DialogProfile::on_scriptFilenameBrowse_clicked()
{
    QString filename(QFileDialog::getOpenFileName(this, tr("Choose Script File"), ui->scriptFilename->text(), tr("Lua Scripts (*.lua);;All files (*)")));
    if (!filename.isEmpty())
    {
        ui->scriptFilename->setText(filename);
    }
}

void DialogProfile::on_fgCommand_clicked()
{
    QColor fg = QColorDialog::getColor(m_profile->commandForeground(), this, tr("Select Command Foreground Color"));
    if (fg.isValid())
    {
        m_fgCommand = fg;
        ui->fgCommand->setStyleSheet(QString("QPushButton { background-color: %1; }")
                                     .arg(m_fgCommand.name()));
        ui->editCommandSample->setStyleSheet(QString("color: %1; background-color: %2;")
                                             .arg(m_fgCommand.name())
                                             .arg(m_bgCommand.name()));
    }
}

void DialogProfile::on_bgCommand_clicked()
{
    QColor bg = QColorDialog::getColor(m_profile->commandBackground(), this, tr("Select Command Background Color"));
    if (bg.isValid())
    {
        m_bgCommand = bg;
        ui->bgCommand->setStyleSheet(QString("QPushButton { background-color: %1; }")
                                     .arg(m_bgCommand.name()));
        ui->editCommandSample->setStyleSheet(QString("color: %1; background-color: %2;")
                                             .arg(m_fgCommand.name())
                                             .arg(m_bgCommand.name()));
    }
}

void DialogProfile::on_fgNote_clicked()
{
    QColor fg = QColorDialog::getColor(m_profile->noteForeground(), this, tr("Select Note Foreground Color"));
    if (fg.isValid())
    {
        m_fgNote = fg;
        ui->fgNote->setStyleSheet(QString("QPushButton { background-color: %1; }")
                                  .arg(m_fgNote.name()));
        ui->editNoteSample->setStyleSheet(QString("color: %1; background-color: %2;")
                                          .arg(m_fgNote.name())
                                          .arg(m_bgNote.name()));
    }
}

void DialogProfile::on_bgNote_clicked()
{
    QColor bg = QColorDialog::getColor(m_profile->noteBackground(), this, tr("Select Note Background Color"));
    if (bg.isValid())
    {
        m_bgNote = bg;
        ui->bgNote->setStyleSheet(QString("QPushButton { background-color: %1; }")
                                  .arg(m_bgNote.name()));
        ui->editNoteSample->setStyleSheet(QString("color: %1; background-color: %2;")
                                          .arg(m_fgNote.name())
                                          .arg(m_bgNote.name()));
    }
}

void DialogProfile::on_checkAutoWrap_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        ui->wrapColumn->setEnabled(false);
        ui->wrapColumn->setValue(m_profile->wrapColumn());
    }
    else
    {
        ui->wrapColumn->setEnabled(true);
    }
}

void DialogProfile::on_loggingDirectoryBrowse_clicked()
{
    QString path(QFileDialog::getExistingDirectory(this, tr("Choose Log Directory"), ui->loggingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    if (!path.isEmpty())
    {
        ui->loggingDirectory->setText(path);
    }
}
