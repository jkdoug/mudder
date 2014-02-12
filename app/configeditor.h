#ifndef CONFIGEDITOR_H
#define CONFIGEDITOR_H

#include "configpage.h"

namespace Ui {
class ConfigEditor;
}

class ConfigEditor : public ConfigPage
{
    Q_OBJECT

public:
    explicit ConfigEditor(ConfigWidget *parent = 0);
    ~ConfigEditor();

public slots:
    virtual void load();
    virtual void save();
    virtual void restoreDefaults();

private:
    Ui::ConfigEditor *ui;
};

#endif // CONFIGEDITOR_H
