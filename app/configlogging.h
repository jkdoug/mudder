#ifndef CONFIGLOGGING_H
#define CONFIGLOGGING_H

#include "configpage.h"

namespace Ui {
class ConfigLogging;
}

class Profile;

class ConfigLogging : public ConfigPage
{
    Q_OBJECT

public:
    explicit ConfigLogging(Profile *profile, ConfigWidget *parent = 0);
    ~ConfigLogging();

public slots:
    virtual void load();
    virtual void save();
    virtual bool validate();

protected slots:
    void on_loggingDirectoryBrowse_clicked();

private:
    Ui::ConfigLogging *ui;

    Profile *m_profile;
};

#endif // CONFIGLOGGING_H
