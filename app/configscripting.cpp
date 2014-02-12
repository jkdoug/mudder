#include "configscripting.h"
#include "ui_configscripting.h"
#include "profile.h"
#include <QFileDialog>

ConfigScripting::ConfigScripting(Profile *profile, ConfigWidget *parent) :
    ConfigPage(parent),
    ui(new Ui::ConfigScripting),
    m_profile(profile)
{
    ui->setupUi(this);
}

ConfigScripting::~ConfigScripting()
{
    delete ui;
}

void ConfigScripting::load()
{
    ui->scriptFilename->setText(m_profile->scriptFilename());
    ui->scriptPrefix->setText(m_profile->scriptPrefix());
}

void ConfigScripting::save()
{
    m_profile->setScriptFilename(ui->scriptFilename->text());
    m_profile->setScriptPrefix(ui->scriptPrefix->text());
}

void ConfigScripting::on_scriptFilenameBrowse_clicked()
{
    QString filename(QFileDialog::getOpenFileName(this, tr("Choose Script File"), ui->scriptFilename->text(), tr("Lua Scripts (*.lua);;All files (*)")));
    if (!filename.isEmpty())
    {
        QFileInfo fi(filename);
        ui->scriptFilename->setText(fi.filePath());
    }
}
