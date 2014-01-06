#ifndef DIALOGALIAS_H
#define DIALOGALIAS_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class DialogAlias;
}

class Alias;

class DialogAlias : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogAlias(Alias *alias, QWidget *parent = 0);
    ~DialogAlias();

public slots:
    void changed();
    virtual void accept();

private:
    Ui::DialogAlias *ui;

    QPushButton *m_ok;

    Alias *m_alias;
};

#endif // DIALOGALIAS_H
