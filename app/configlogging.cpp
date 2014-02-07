#include "configlogging.h"
#include "ui_configlogging.h"
#include "profile.h"
#include <QFileDialog>
#include <QMessageBox>

ConfigLogging::ConfigLogging(Profile *profile, ConfigWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigLogging),
    m_profile(profile)
{
    ui->setupUi(this);

    connect(parent, SIGNAL(loading()), SLOT(load()));
    connect(parent, SIGNAL(saving()), SLOT(save()));
}

ConfigLogging::~ConfigLogging()
{
    delete ui;
}

void ConfigLogging::load()
{
    ui->loggingDirectory->setText(m_profile->logDirectory());
    ui->loggingFilename->setText(m_profile->logFilename());
    ui->loggingAutoStart->setChecked(m_profile->autoLog());
}

void ConfigLogging::save()
{
    m_profile->setLogDirectory(ui->loggingDirectory->text());
    m_profile->setLogFilename(ui->loggingFilename->text());
    m_profile->setAutoLog(ui->loggingAutoStart->isChecked());
}

bool ConfigLogging::validate()
{
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

void ConfigLogging::on_loggingDirectoryBrowse_clicked()
{
    QString path(QFileDialog::getExistingDirectory(this, tr("Choose Log Directory"), ui->loggingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    if (!path.isEmpty())
    {
        ui->loggingDirectory->setText(path);
    }
}
