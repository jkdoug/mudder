#include <QDebug>
#include "configeditor.h"
#include "ui_configeditor.h"
#include "options.h"

ConfigEditor::ConfigEditor(ConfigWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigEditor)
{
    ui->setupUi(this);

    QStringList fontSizes;
    for (int n = 6; n <= 30; n++)
    {
        fontSizes << QString::number(n);
    }
    ui->comboEditorFontSize->addItems(fontSizes);
}

ConfigEditor::~ConfigEditor()
{
    delete ui;
}

bool ConfigEditor::validate()
{
    return true;
}

void ConfigEditor::load()
{
    QFont font(OPTIONS->editorFont());

    ui->comboEditorFont->setCurrentFont(font);
    ui->comboEditorFontSize->setCurrentIndex(font.pointSize() - 6);
    ui->checkEditorAntiAliased->setChecked(font.styleStrategy() & QFont::PreferAntialias);
}

void ConfigEditor::save()
{
    bool antiAlias = ui->checkEditorAntiAliased->isChecked();

    QFont font(ui->comboEditorFont->currentFont());
    font.setStyleHint(QFont::TypeWriter, antiAlias?QFont::PreferAntialias:QFont::NoAntialias);
    font.setPointSize(ui->comboEditorFontSize->currentText().toInt());

    OPTIONS->setEditorFont(font);
}

void ConfigEditor::restoreDefaults()
{
    QFont font(OPTIONS->editorFont(true));

    ui->comboEditorFont->setCurrentFont(font);
    ui->comboEditorFontSize->setCurrentIndex(font.pointSize() - 6);
    ui->checkEditorAntiAliased->setChecked(font.styleStrategy() & QFont::PreferAntialias);
}
