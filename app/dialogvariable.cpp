#include "dialogvariable.h"
#include "ui_dialogvariable.h"
#include "variable.h"
#include <QMessageBox>

DialogVariable::DialogVariable(Variable *variable, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogVariable)
{
    ui->setupUi(this);

    Q_ASSERT(variable != 0);
    m_variable = variable;

    ui->name->setText(m_variable->name());

    ui->type->addItems(QStringList() << "string" << "numeric" << "boolean");
    switch (m_variable->contents().type())
    {
        case QVariant::Bool:
            ui->type->setCurrentIndex(2);
            break;

        case QVariant::Double:
            ui->type->setCurrentIndex(1);
            break;

        default:
            ui->type->setCurrentIndex(0);
            break;
    }

    ui->contents->setPlainText(m_variable->value());

    m_ok = ui->buttonBox->button(QDialogButtonBox::Ok);

    changed();

    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(ui->contents, SIGNAL(textChanged()), this, SLOT(changed()));
}

DialogVariable::~DialogVariable()
{
    delete ui;
}

void DialogVariable::changed()
{
    bool changed = m_variable->name() != ui->name->text() ||
            m_variable->value() != ui->contents->toPlainText();

    QString type = ui->type->itemText(ui->type->currentIndex());
    QVariant::Type enumType = Variable::translateType(type);

    QVariant val(ui->contents->toPlainText().trimmed());
    bool valid = !ui->name->text().isEmpty() &&
            val.convert(enumType);

    m_ok->setEnabled(changed && valid);
}

void DialogVariable::accept()
{
    QString type = ui->type->itemText(ui->type->currentIndex());
    QVariant::Type enumType = Variable::translateType(type);

    QVariant val(ui->contents->toPlainText().trimmed());
    if (!val.convert(enumType))
    {
        QMessageBox::critical(this, tr("Invalid Variable"), tr("The selected data type does not allow this data to be stored."));
        return;
    }

    m_variable->setName(ui->name->text());

    m_variable->setContents(val);

    QDialog::accept();
}
