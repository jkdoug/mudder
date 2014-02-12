#ifndef CONFIGCONNECTION_H
#define CONFIGCONNECTION_H

#include "configpage.h"

namespace Ui {
class ConfigConnection;
}

class Profile;

class ConfigConnection : public ConfigPage
{
    Q_OBJECT

public:
    explicit ConfigConnection(Profile *profile, ConfigWidget *parent = 0);
    ~ConfigConnection();

public slots:
    virtual void load();
    virtual void save();
    virtual bool validate();

private:
    Ui::ConfigConnection *ui;

    Profile *m_profile;
};

#endif // CONFIGCONNECTION_H
