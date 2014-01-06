#include "dialoggroup.h"
#include "ui_dialoggroup.h"
#include "group.h"

DialogGroup::DialogGroup(Group *group, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGroup)
{
    ui->setupUi(this);

    Q_ASSERT(group != 0);
    m_group = group;

    ui->name->setText(m_group->name());

    ui->enabled->setChecked(m_group->enabledFlag());

    m_ok = ui->buttonBox->button(QDialogButtonBox::Ok);

    changed();

    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), this, SLOT(changed()));
}

DialogGroup::~DialogGroup()
{
    delete ui;
}

void DialogGroup::changed()
{
    bool changed = m_group->name() != ui->name->text() ||
            m_group->enabledFlag() != ui->enabled->isChecked();

    bool valid = !ui->name->text().isEmpty();

    m_ok->setEnabled(changed && valid);
}

void DialogGroup::accept()
{
    m_group->setName(ui->name->text());

    m_group->enable(ui->enabled->isChecked());

    QDialog::accept();
}
