#include "configconnection.h"
#include "ui_configconnection.h"
#include "profile.h"
#include <QMessageBox>

ConfigConnection::ConfigConnection(Profile *profile, ConfigWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigConnection),
    m_profile(profile)
{
    ui->setupUi(this);

    connect(parent, SIGNAL(loading()), SLOT(load()));
    connect(parent, SIGNAL(saving()), SLOT(save()));
}

ConfigConnection::~ConfigConnection()
{
    delete ui;
}

void ConfigConnection::load()
{
    ui->name->setText(m_profile->name());
    ui->address->setText(m_profile->address());
    ui->port->setText(QString::number(m_profile->port()));

    ui->checkAutoConnect->setChecked(m_profile->autoConnect());
    ui->checkReconnect->setChecked(m_profile->autoReconnect());
    ui->checkClearCommands->setChecked(m_profile->clearCommandAfterSend());
}

void ConfigConnection::save()
{
    m_profile->setName(ui->name->text());
    m_profile->setAddress(ui->address->text());
    m_profile->setPort(ui->port->text().toInt());

    m_profile->setAutoConnect(ui->checkAutoConnect->isChecked());
    m_profile->setAutoReconnect(ui->checkReconnect->isChecked());
    m_profile->setClearCommandAfterSend(ui->checkClearCommands->isChecked());
}

bool ConfigConnection::validate()
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

    return true;
}
