#ifndef DIALOGVARIABLE_H
#define DIALOGVARIABLE_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class DialogVariable;
}

class Variable;

class DialogVariable : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogVariable(Variable *variable, QWidget *parent = 0);
    ~DialogVariable();

public slots:
    void changed();
    virtual void accept();

private:
    Ui::DialogVariable *ui;

    QPushButton *m_ok;

    Variable *m_variable;
};

#endif // DIALOGVARIABLE_H
