#ifndef CONFIGSCRIPTING_H
#define CONFIGSCRIPTING_H

#include <QWidget>
#include "configwidget.h"

namespace Ui {
class ConfigScripting;
}

class Profile;

class ConfigScripting : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigScripting(Profile *profile, ConfigWidget *parent = 0);
    ~ConfigScripting();

private slots:
    void load();
    void save();

    void on_scriptFilenameBrowse_clicked();

private:
    Ui::ConfigScripting *ui;

    Profile *m_profile;
};

#endif // CONFIGSCRIPTING_H
