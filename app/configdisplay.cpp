#include "configdisplay.h"
#include "ui_configdisplay.h"
#include "profile.h"
#include <QColorDialog>

ConfigDisplay::ConfigDisplay(Profile *profile, ConfigWidget *parent) :
    ConfigPage(parent),
    ui(new Ui::ConfigDisplay),
    m_profile(profile)
{
    ui->setupUi(this);

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
}

ConfigDisplay::~ConfigDisplay()
{
    delete ui;
}

void ConfigDisplay::load()
{
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
}

void ConfigDisplay::save()
{
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
}

void ConfigDisplay::on_fgCommand_clicked()
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

void ConfigDisplay::on_bgCommand_clicked()
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

void ConfigDisplay::on_fgNote_clicked()
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

void ConfigDisplay::on_bgNote_clicked()
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

void ConfigDisplay::on_checkAutoWrap_stateChanged(int arg1)
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
