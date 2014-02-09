#ifndef CONFIGEDITOR_H
#define CONFIGEDITOR_H

#include <QWidget>
#include "configwidget.h"

namespace Ui {
class ConfigEditor;
}

class ConfigEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigEditor(ConfigWidget *parent = 0);
    ~ConfigEditor();

private slots:
    void load();
    void save();
    bool validate();
    void restoreDefaults();

private:
    Ui::ConfigEditor *ui;
};

#endif // CONFIGEDITOR_H
