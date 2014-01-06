#ifndef DIALOGGROUP_H
#define DIALOGGROUP_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class DialogGroup;
}

class Group;

class DialogGroup : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogGroup(Group *group, QWidget *parent = 0);
    ~DialogGroup();

public slots:
    void changed();
    virtual void accept();

private:
    Ui::DialogGroup *ui;

    QPushButton *m_ok;

    Group *m_group;
};

#endif // DIALOGGROUP_H
