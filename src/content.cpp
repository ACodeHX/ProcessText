#include "./content.h"
#include "./ui_content.h"

Content::Content(
    QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Content)
{
    ui->setupUi(this);
    connect(ui->confirmButton, &QPushButton::clicked, this, &Content::accept);
}

Content::~Content()
{
    delete ui;
}

QString Content::confirmContent() {
    accept();
    return ui->line_text->text();
}

QString Content::insertPosition() {
    qDebug() << "before checked:" << ui->before->isChecked();
    qDebug() << "back checked:" << ui->back->isChecked();
    qDebug() << "both_end checked:" << ui->both_end->isChecked();

    if (ui->before->isChecked()) {
        return "before";
    } else if (ui->back->isChecked()) {
        return "back";
    } else if (ui->both_end->isChecked()) {
        return "both end";
    }
    return "error";
}
