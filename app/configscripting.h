#ifndef CONFIGSCRIPTING_H
#define CONFIGSCRIPTING_H

#include "configpage.h"

namespace Ui {
class ConfigScripting;
}

class Profile;

class ConfigScripting : public ConfigPage
{
    Q_OBJECT

public:
    explicit ConfigScripting(Profile *profile, ConfigWidget *parent = 0);
    ~ConfigScripting();

public slots:
    virtual void load();
    virtual void save();

protected slots:
    void on_scriptFilenameBrowse_clicked();

private:
    Ui::ConfigScripting *ui;

    Profile *m_profile;
};

#endif // CONFIGSCRIPTING_H
